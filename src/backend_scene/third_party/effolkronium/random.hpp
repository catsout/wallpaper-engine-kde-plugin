/*
______  ___   _   _______ ________  __
| ___ \/ _ \ | \ | |  _  \  _  |  \/  | Random for modern C++
| |_/ / /_\ \|  \| | | | | | | | .  . |
|    /|  _  || . ` | | | | | | | |\/| | version 1.4.1
| |\ \| | | || |\  | |/ /\ \_/ / |  | |
\_| \_\_| |_/\_| \_/___/  \___/\_|  |_/ https://github.com/effolkronium/random

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2017-2022 effolkronium (Illia Polishchuk)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files( the "Software" ), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef EFFOLKRONIUM_RANDOM_HPP
#define EFFOLKRONIUM_RANDOM_HPP

#include <random>
#include <chrono> // timed seed
#include <type_traits>
#include <cassert>
#include <initializer_list>
#include <utility> // std::forward, std::declval
#include <algorithm> // std::shuffle, std::next, std::distance
#include <iterator> // std::begin, std::end, std::iterator_traits
#include <limits> // std::numeric_limits
#include <ostream>
#include <istream>

namespace effolkronium {

    namespace details {
        /// Key type for getting common type numbers or objects
        struct common{ }; 

        /// True if type T is applicable by a std::uniform_int_distribution
        template<typename T>
        struct is_uniform_int {
            static constexpr bool value =
                   std::is_same<T,              short>::value
                || std::is_same<T,                int>::value
                || std::is_same<T,               long>::value
                || std::is_same<T,          long long>::value
                || std::is_same<T,     unsigned short>::value
                || std::is_same<T,       unsigned int>::value
                || std::is_same<T,      unsigned long>::value
                || std::is_same<T, unsigned long long>::value;
        };

        /// True if type T is applicable by a std::uniform_real_distribution
        template<typename T>
        struct is_uniform_real {
            static constexpr bool value =
                   std::is_same<T,       float>::value
                || std::is_same<T,      double>::value
                || std::is_same<T, long double>::value;
        };

        /// True if type T is plain byte
        template<typename T>
        struct is_byte {
            static constexpr bool value =
                   std::is_same<T,   signed char>::value
                || std::is_same<T, unsigned char>::value;
        };

        /// True if type T is plain number type
        template<typename T>
        struct is_supported_number {
            static constexpr bool value =
                   is_byte        <T>::value
                || is_uniform_real<T>::value
                || is_uniform_int <T>::value;
        };

        /// True if type T is character type
        template<typename T>
        struct is_supported_character {
            static constexpr bool value =
                   std::is_same<T, char>::value
                || std::is_same<T, wchar_t>::value
                || std::is_same<T, char16_t>::value
                || std::is_same<T, char32_t>::value;
        };

        /// True if type T is iterator
        template<typename T>
        struct is_iterator {
        private:
            static char test( ... );

            template <typename U,
                typename = typename std::iterator_traits<U>::difference_type,
                typename = typename std::iterator_traits<U>::pointer,
                typename = typename std::iterator_traits<U>::reference,
                typename = typename std::iterator_traits<U>::value_type,
                typename = typename std::iterator_traits<U>::iterator_category
            > static long test( U&& );
        public:
            static constexpr bool value = std::is_same<
                decltype( test( std::declval<T>( ) ) ), long>::value;
        };

        template <typename T>
        class has_reserve
        {
        private:
            template <typename C>
            static char test(...);

            template <typename C>
            static long test(decltype(&C::reserve));
        public:
            static constexpr bool value = std::is_same<
                decltype(test<T>(0)), long>::value;
        };

        template <typename T>
        class has_insert
        {
        private:
            template <typename C>
            static char test(...);

            template <typename C>
            static long test(decltype(&C::insert));
        public:
            static constexpr bool value = std::is_same<
                decltype(test<T>(0)), long>::value;
        };

    } // namespace details

    /// Default seeder for 'random' classes
    struct seeder_default {
        /// return seed sequence
        std::seed_seq& operator() ( ) {
            // MinGW issue, std::random_device returns constant value
            // Use std::seed_seq with additional seed from C++ chrono
            return seed_seq;
        }
    private:
        std::seed_seq seed_seq{ {
                static_cast<std::uintmax_t>( std::random_device{ }( ) ),
                static_cast<std::uintmax_t>( std::chrono::steady_clock::now( )
                                             .time_since_epoch( ).count( ) ),
        } };
    };

    /**
    * \brief Base template class for random 
    *        with static API and static internal member storage
    * \note it is NOT thread safe but more efficient then 
    *                           basic_random_thread_local
    * \param Engine A random engine with interface like in the std::mt19937
    * \param Seeder A seeder type which return seed for internal engine 
    *                                             through operator()
    */
    template<
        typename Engine,
        typename Seeder = seeder_default,
        template<typename> class IntegerDist = std::uniform_int_distribution,
        template<typename> class RealDist = std::uniform_real_distribution,
        typename BoolDist = std::bernoulli_distribution
    >
    class basic_random_static {
    public:
        basic_random_static( ) = delete;

        /// Type of used random number engine
        using engine_type = Engine;

        /// Type of used random number seeder
        using seeder_type = Seeder;

        /// Type of used integer distribution
        template<typename T>
        using integer_dist_t = IntegerDist<T>;

        /// Type of used real distribution
        template<typename T>
        using real_dist_t = RealDist<T>;

        /// Type of used bool distribution
        using bool_dist_t = BoolDist;

        /// Key type for getting common type numbers or objects
        using common = details::common;

        /**
        * \return The minimum value
        * potentially generated by the random-number engine
        */
        static constexpr typename Engine::result_type (min)( ) {
            return (Engine::min)( );
        }

        /**
        * \return The maximum value
        * potentially generated by the random-number engine
        */
        static constexpr typename Engine::result_type (max)( ) {
            return (Engine::max)( );
        }

        /// Advances the internal state by z times
        static void discard( const unsigned long long z ) {
            engine_instance( ).discard( z );
        }

        /// Reseed by Seeder
        static void reseed( ) {
            Seeder seeder;
            seed( seeder( ) );
        }

        /**
        * \brief Reinitializes the internal state
        * of the random-number engine using new seed value
        * \param value The seed value to use 
        *        in the initialization of the internal state
        */
        static void seed( const typename Engine::result_type value =
                          Engine::default_seed ) {
            engine_instance( ).seed( value );
        }

        /**
        * \brief Reinitializes the internal state
        * of the random-number engine using new seed value
        * \param seq The seed sequence 
        *        to use in the initialization of the internal state
        */
        template<typename Sseq>
        static void seed( Sseq& seq ) {
            engine_instance( ).seed( seq );
        }

        /// return random number from engine in [min(), max()] range
        static typename Engine::result_type get( ) {
            return engine_instance( )( );
        }

        /**
        * \brief Compares internal pseudo-random number engine
        *        with 'other' pseudo-random number engine.
        *        Two engines are equal, if their internal states
        *        are equivalent, that is, if they would generate
        *        equivalent values for any number of calls of operator()
        * \param other The engine, with which the internal engine will be compared
        * \return true, if other and internal engine are equal
        */
        static bool is_equal( const Engine& other ) {
            return engine_instance( ) == other;
        }

        /**
        * \brief Serializes the internal state of the
        *        internal pseudo-random number engine as a sequence
        *        of decimal numbers separated by one or more spaces,
        *        and inserts it to the stream ost. The fill character
        *        and the formatting flags of the stream are
        *        ignored and unaffected.
        * \param ost The output stream to insert the data to
        */
        template<typename CharT, typename Traits>
        static void serialize( std::basic_ostream<CharT, Traits>& ost ) {
            ost << engine_instance( );
        }

        /**
        * \brief Restores the internal state of the
        *        internal pseudo-random number engine from
        *        the serialized representation, which
        *        was created by an earlier call to 'serialize'
        *        using a stream with the same imbued locale and
        *        the same CharT and Traits.
        *        If the input cannot be deserialized,
        *        internal engine is left unchanged and failbit is raised on ist
        * \param ost The input stream to extract the data from
        */
        template<typename CharT, typename Traits>
        static void deserialize( std::basic_istream<CharT, Traits>& ist ) {
            ist >> engine_instance( );
        }

        /**
        * \brief Generate a random integer number in a [from; to] range
        *        by std::uniform_int_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random integer number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_uniform_int<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            if( from < to ) // Allow range from higher to lower
                return IntegerDist<T>{ from, to }( engine_instance( ) );
            return IntegerDist<T>{ to, from }( engine_instance( ) );
        }

        /**
        * \brief Generate a random real number in a [from; to] range
        *        by std::uniform_real_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random real number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_uniform_real<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            if( from < to ) // Allow range from higher to lower
                return RealDist<T>{ from, to }( engine_instance( ) );
            return RealDist<T>{ to, from }( engine_instance( ) );
        }

        /**
        * \brief Generate a random byte number in a [from; to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random byte number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_byte<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            // Choose between short and unsigned short for byte conversion
            using short_t = typename std::conditional<std::is_signed<T>::value,
                short, unsigned short>::type;

            return static_cast<T>( get<short_t>( from, to ) );
        }

        /**
        * \brief Generate a random common_type number in a [from; to] range
        * \param Key The Key type for this version of 'get' method
        *        Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random common_type number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Allow implicit type conversion
        * \note Prevent implicit type conversion from singed to unsigned types
        *       Why? std::common_type<Unsigned, Signed> chooses unsigned value,
        *                 then Signed value will be converted to Unsigned value
        *                       which gives us a wrong range for random values.
        *                           https://stackoverflow.com/a/5416498/5734836
        */
        template<
            typename Key,
            typename A,
            typename B, 
            typename C = typename std::common_type<A, B>::type
        >
        static typename std::enable_if<
               std::is_same<Key, common>::value
            && details::is_supported_number<A>::value
            && details::is_supported_number<B>::value
            // Prevent implicit type conversion from singed to unsigned types
            && std::is_signed<A>::value != std::is_unsigned<B>::value
            , C>::type get( A from = (std::numeric_limits<A>::min)( ),
                            B to = (std::numeric_limits<B>::max)( ) ) {
            return get( static_cast<C>( from ), static_cast<C>( to ) );
        }

        /**
        * \brief Generate a random character in a [from; to] range
        *        by std::uniform_int_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random character in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_supported_character<T>::value
            , T>::type get(T from = (std::numeric_limits<T>::min)(),
                T to = (std::numeric_limits<T>::max)()) {
            if (from < to) // Allow range from higher to lower
                return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(from), static_cast<std::int64_t>(to) }(engine_instance()));
            return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(to), static_cast<std::int64_t>(from) }(engine_instance()));
        }

        /**
        * \brief Generate a bool value with specific probability
        *                         by std::bernoulli_distribution
        * \param probability The probability of generating true in [0; 1] range
        *        0 means always false, 1 means always true
        * \return 'true' with 'probability' probability ('false' otherwise)
        */
        template<typename T>
        static typename std::enable_if<std::is_same<T, bool>::value
            , bool>::type get( const double probability = 0.5 ) {
            assert( 0 <= probability && 1 >= probability ); // out of [0; 1] range
            return BoolDist{ probability }( engine_instance( ) );
        }

        /**
        * \brief Return random value from initilizer_list
        * \param init_list initilizer_list with values
        * \return Random value from initilizer_list
        * \note Should be 1 or more elements in initilizer_list
        * \note Warning! Elements in initilizer_list can't be moved:
        *               https://stackoverflow.com/a/8193157/5734836
        */
        template<typename T>
        static T get( std::initializer_list<T> init_list ) {
            assert( 0u != init_list.size( ) );
            return *get( init_list.begin( ), init_list.end( ) );
        }

        /**
        * \brief Return random iterator from iterator range
        * \param first, last - the range of elements
        * \return Random iterator from [first, last) range
        * \note If first == last, return last
        */
        template<typename InputIt>
        static typename std::enable_if<details::is_iterator<InputIt>::value
            , InputIt>::type get( InputIt first, InputIt last ) {
            const auto size = std::distance( first, last );
            if( 0 == size ) return last;
            using diff_t = typename std::iterator_traits<InputIt>::difference_type;
            return std::next( first, get<diff_t>( 0, size - 1 ) );
        }

        /**
        * \brief Return random iterator from Container
        * \param container The container with elements
        * \return Random iterator from container
        * \note If container is empty return std::end( container ) iterator
        */
        template<typename Container>
        static auto get( Container& container ) -> 
            typename std::enable_if<details::is_iterator<
                decltype(std::begin(container))>::value
                , decltype(std::begin(container))
            >::type {
            return get( std::begin( container ), std::end( container ) );
        }

        /**
        * \brief Return container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        * \note Container "reserve" method will be called before generation
        */
        template<template<typename...> class Container, typename A>
        static typename std::enable_if<
                details::has_reserve<Container<A>>::value
            , Container<A>>::type get(A from, A to, std::size_t size) {
            Container<A> container;

            container.reserve(size);
            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get(from, to));

            return container;
        }

        /**
        * \brief Return container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        * \note Container "reserve" method will be called before generation
        */
        template<
            template<typename...> class Container,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
                   std::is_same<Key, common>::value
                && details::has_reserve<Container<A>>::value
            , Container<C>>::type get(A start, B end, std::size_t size) {
            Container<C> container;

            container.reserve(size);
            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get<common>(start, end));

            return container;
        }

        /**
        * \brief Return container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<template<typename...> class Container, typename A>
        static typename std::enable_if<
              !details::has_reserve<Container<A>>::value
            , Container<A>>::type get(A start, A end, std::size_t size) {
            Container<A> container;

            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get(start, end));

            return container;
        }

        /**
        * \brief Return container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<
            template<typename...> class Container,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
                 std::is_same<Key, common>::value
              && !details::has_reserve<Container<C>>::value
            , Container<C>>::type get(A start, B end, std::size_t size) {
            Container<C> container;

            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get<common>(start, end));

            return container;
        }

        /**
        * \brief Return array-like container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param N The number of elements in resulting container
        * \return Container filled with random values
        */
        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
        static typename std::enable_if<
               !details::has_insert<Container<A, N>>::value
            , Container<A, N>>::type get(A start, A end) {
            Container<A, N> container = {{ 0 }};

            for (std::size_t i = 0; i < N; ++i)
                container[i] = get(start, end);

            return container;
        }

        /**
        * \brief Return array-like container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<
            template<typename AA, std::size_t NN, typename...> class Container,
            std::size_t N,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
                  std::is_same<Key, common>::value
               && !details::has_insert<Container<C, N>>::value
            , Container<C, N>>::type get(A start, B end) {
            Container<C, N> container = {{ 0 }};

            for (std::size_t i = 0; i < N; ++i)
                container[i] = get<common>(start, end);

            return container;
        }

        /**
        * \brief Return random pointer from built-in array
        * \param array The built-in array with elements
        * \return Pointer to random element in array
        */
        template<typename T, std::size_t N>
        static T* get( T( &array )[ N ] ) {
            return std::addressof( array[ get<std::size_t>( 0, N - 1 ) ] );
        }

        /**
        * \brief Return value from custom Dist distribution
        *        seeded by internal random engine
        * \param Dist The type of custom distribution with next concept:
        *        http://en.cppreference.com/w/cpp/concept/RandomNumberDistribution
        * \param args The arguments which will be forwarded to Dist constructor
        * \return Value from custom distribution
        */
        template<typename Dist, typename... Args>
        static typename Dist::result_type get( Args&&... args ) {
            return Dist{ std::forward<Args>( args )... }( engine_instance( ) );
        }

        /**
        * \brief Return value from custom 'dist' distribution
        *        seeded by internal random engine
        * \param dist The custom distribution with next concept:
        *        http://en.cppreference.com/w/cpp/concept/RandomNumberDistribution
        * \param args The arguments which will be forwarded to Dist constructor
        * \return Value from custom 'dist' distribution
        */
        template<typename Dist>
        static typename Dist::result_type get( Dist& dist ) {
            return dist( engine_instance( ) );
        }

        /**
        * \brief Reorders the elements in the given range [first, last)
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param first, last - the range of elements to shuffle randomly       
        */
        template<typename RandomIt>
        static void shuffle( RandomIt first, RandomIt last ) {
            std::shuffle( first, last, engine_instance( ) );
        }

        /**
        * \brief Reorders the elements in the given container
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param container - the container with elements to shuffle randomly
        */
        template<typename Container>
        static void shuffle( Container& container ) {
            shuffle( std::begin( container ), std::end( container ) );
        }

        /// return internal engine by copy
        static Engine get_engine( ) {
            return engine_instance( );
        }

        /// return internal engine by ref
        static Engine& engine() {
            return engine_instance();
        }
    protected:
        /// get reference to the static engine instance
        static Engine& engine_instance( ) {
            static Engine engine{ Seeder{ }( ) };
            return engine;
        }
    };

    /**
    * \brief Base template class for random 
    *        with thread_local API and thread_local internal member storage
    * \note it IS thread safe but less efficient then 
    *                           basic_random_static
    * \param Engine A random engine with interface like in the std::mt19937
    * \param Seeder A seeder type which return seed for internal engine 
    *                                             through operator()
    */
    template<
        typename Engine,
        typename Seeder = seeder_default,
        template<typename> class IntegerDist = std::uniform_int_distribution,
        template<typename> class RealDist = std::uniform_real_distribution,
        typename BoolDist = std::bernoulli_distribution
    >
    class basic_random_thread_local {
    public:
        basic_random_thread_local( ) = delete;

        /// Type of used random number engine
        using engine_type = Engine;

        /// Type of used random number seeder
        using seeder_type = Seeder;

        /// Type of used integer distribution
        template<typename T>
        using integer_dist_t = IntegerDist<T>;

        /// Type of used real distribution
        template<typename T>
        using real_dist_t = RealDist<T>;

        /// Type of used bool distribution
        using bool_dist_t = BoolDist;

        /// Key type for getting common type numbers or objects
        using common = details::common;

        /**
        * \return The minimum value
        * potentially generated by the random-number engine
        */
        static constexpr typename Engine::result_type (min)( ) {
            return (Engine::min)( );
        }

        /**
        * \return The maximum value
        * potentially generated by the random-number engine
        */
        static constexpr typename Engine::result_type (max)( ) {
            return (Engine::max)( );
        }

        /// Advances the internal state by z times
        static void discard( const unsigned long long z ) {
            engine_instance( ).discard( z );
        }

        /// Reseed by Seeder
        static void reseed( ) {
            Seeder seeder;
            seed( seeder( ) );
        }

        /**
        * \brief Reinitializes the internal state
        * of the random-number engine using new seed value
        * \param value The seed value to use 
        *        in the initialization of the internal state
        */
        static void seed( const typename Engine::result_type value =
                          Engine::default_seed ) {
            engine_instance( ).seed( value );
        }

        /**
        * \brief Reinitializes the internal state
        * of the random-number engine using new seed value
        * \param seq The seed sequence 
        *        to use in the initialization of the internal state
        */
        template<typename Sseq>
        static void seed( Sseq& seq ) {
            engine_instance( ).seed( seq );
        }

        /// return random number from engine in [min(), max()] range
        static typename Engine::result_type get( ) {
            return engine_instance( )( );
        }

        /**
        * \brief Compares internal pseudo-random number engine
        *        with 'other' pseudo-random number engine.
        *        Two engines are equal, if their internal states
        *        are equivalent, that is, if they would generate
        *        equivalent values for any number of calls of operator()
        * \param other The engine, with which the internal engine will be compared
        * \return true, if other and internal engine are equal
        */
        static bool is_equal( const Engine& other ) {
            return engine_instance( ) == other;
        }

        /**
        * \brief Serializes the internal state of the
        *        internal pseudo-random number engine as a sequence
        *        of decimal numbers separated by one or more spaces,
        *        and inserts it to the stream ost. The fill character
        *        and the formatting flags of the stream are
        *        ignored and unaffected.
        * \param ost The output stream to insert the data to
        */
        template<typename CharT, typename Traits>
        static void serialize( std::basic_ostream<CharT, Traits>& ost ) {
            ost << engine_instance( );
        }

        /**
        * \brief Restores the internal state of the
        *        internal pseudo-random number engine from
        *        the serialized representation, which
        *        was created by an earlier call to 'serialize'
        *        using a stream with the same imbued locale and
        *        the same CharT and Traits.
        *        If the input cannot be deserialized,
        *        internal engine is left unchanged and failbit is raised on ist
        * \param ost The input stream to extract the data from
        */
        template<typename CharT, typename Traits>
        static void deserialize( std::basic_istream<CharT, Traits>& ist ) {
            ist >> engine_instance( );
        }

        /**
        * \brief Generate a random integer number in a [from; to] range
        *        by std::uniform_int_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random integer number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_uniform_int<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            if( from < to ) // Allow range from higher to lower
                return IntegerDist<T>{ from, to }( engine_instance( ) );
            return IntegerDist<T>{ to, from }( engine_instance( ) );
        }

        /**
        * \brief Generate a random real number in a [from; to] range
        *        by std::uniform_real_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random real number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_uniform_real<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            if( from < to ) // Allow range from higher to lower
                return RealDist<T>{ from, to }( engine_instance( ) );
            return RealDist<T>{ to, from }( engine_instance( ) );
        }

        /**
        * \brief Generate a random byte number in a [from; to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random byte number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_byte<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            // Choose between short and unsigned short for byte conversion
            using short_t = typename std::conditional<std::is_signed<T>::value,
                short, unsigned short>::type;

            return static_cast<T>( get<short_t>( from, to ) );
        }

        /**
        * \brief Generate a random common_type number in a [from; to] range
        * \param Key The Key type for this version of 'get' method
        *        Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random common_type number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Allow implicit type conversion
        * \note Prevent implicit type conversion from singed to unsigned types
        *       Why? std::common_type<Unsigned, Signed> chooses unsigned value,
        *                 then Signed value will be converted to Unsigned value
        *                       which gives us a wrong range for random values.
        *                           https://stackoverflow.com/a/5416498/5734836
        */
        template<
            typename Key,
            typename A,
            typename B, 
            typename C = typename std::common_type<A, B>::type
        >
        static typename std::enable_if<
               std::is_same<Key, common>::value
            && details::is_supported_number<A>::value
            && details::is_supported_number<B>::value
            // Prevent implicit type conversion from singed to unsigned types
            && std::is_signed<A>::value != std::is_unsigned<B>::value
            , C>::type get( A from = (std::numeric_limits<A>::min)( ),
                            B to = (std::numeric_limits<B>::max)( ) ) {
            return get( static_cast<C>( from ), static_cast<C>( to ) );
        }

        /**
        * \brief Generate a random character in a [from; to] range
        *        by std::uniform_int_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random character in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        static typename std::enable_if<details::is_supported_character<T>::value
            , T>::type get(T from = (std::numeric_limits<T>::min)(),
                T to = (std::numeric_limits<T>::max)()) {
            if (from < to) // Allow range from higher to lower
                return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(from), static_cast<std::int64_t>(to) }(engine_instance()));
            return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(to), static_cast<std::int64_t>(from) }(engine_instance()));
        }

        /**
        * \brief Generate a bool value with specific probability
        *                         by std::bernoulli_distribution
        * \param probability The probability of generating true in [0; 1] range
        *        0 means always false, 1 means always true
        * \return 'true' with 'probability' probability ('false' otherwise)
        */
        template<typename T>
        static typename std::enable_if<std::is_same<T, bool>::value
            , bool>::type get( const double probability = 0.5 ) {
            assert( 0 <= probability && 1 >= probability ); // out of [0; 1] range
            return BoolDist{ probability }( engine_instance( ) );
        }

        /**
        * \brief Return random value from initilizer_list
        * \param init_list initilizer_list with values
        * \return Random value from initilizer_list
        * \note Should be 1 or more elements in initilizer_list
        * \note Warning! Elements in initilizer_list can't be moved:
        *               https://stackoverflow.com/a/8193157/5734836
        */
        template<typename T>
        static T get( std::initializer_list<T> init_list ) {
            assert( 0u != init_list.size( ) );
            return *get( init_list.begin( ), init_list.end( ) );
        }

        /**
        * \brief Return random iterator from iterator range
        * \param first, last - the range of elements
        * \return Random iterator from [first, last) range
        * \note If first == last, return last
        */
        template<typename InputIt>
        static typename std::enable_if<details::is_iterator<InputIt>::value
            , InputIt>::type get( InputIt first, InputIt last ) {
            const auto size = std::distance( first, last );
            if( 0 == size ) return last;
            using diff_t = typename std::iterator_traits<InputIt>::difference_type;
            return std::next( first, get<diff_t>( 0, size - 1 ) );
        }

        /**
        * \brief Return random iterator from Container
        * \param container The container with elements
        * \return Random iterator from container
        * \note If container is empty return std::end( container ) iterator
        */
        template<typename Container>
        static auto get( Container& container ) -> 
            typename std::enable_if<details::is_iterator<
                decltype(std::begin(container))>::value
                , decltype(std::begin(container))
            >::type {
            return get( std::begin( container ), std::end( container ) );
        }

        /**
        * \brief Return container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        * \note Container "reserve" method will be called before generation
        */
        template<template<typename...> class Container, typename A>
        static typename std::enable_if<
                details::has_reserve<Container<A>>::value
            , Container<A>>::type get(A from, A to, std::size_t size) {
            Container<A> container;

            container.reserve(size);
            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get(from, to));

            return container;
        }

        /**
        * \brief Return container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        * \note Container "reserve" method will be called before generation
        */
        template<
            template<typename...> class Container,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
                   std::is_same<Key, common>::value
                && details::has_reserve<Container<A>>::value
            , Container<C>>::type get(A start, B end, std::size_t size) {
            Container<C> container;

            container.reserve(size);
            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get<common>(start, end));

            return container;
        }

        /**
        * \brief Return container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<template<typename...> class Container, typename A>
        static typename std::enable_if<
              !details::has_reserve<Container<A>>::value
            , Container<A>>::type get(A start, A end, std::size_t size) {
            Container<A> container;

            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get(start, end));

            return container;
        }

        /**
        * \brief Return container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<
            template<typename...> class Container,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
                 std::is_same<Key, common>::value
              && !details::has_reserve<Container<C>>::value
            , Container<C>>::type get(A start, B end, std::size_t size) {
            Container<C> container;

            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get<common>(start, end));

            return container;
        }

        /**
        * \brief Return array-like container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param N The number of elements in resulting container
        * \return Container filled with random values
        */
        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
        static typename std::enable_if<
               !details::has_insert<Container<A, N>>::value
            , Container<A, N>>::type get(A start, A end) {
            Container<A, N> container = {{ 0 }};

            for (std::size_t i = 0; i < N; ++i)
                container[i] = get(start, end);

            return container;
        }

        /**
        * \brief Return array-like container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<
            template<typename AA, std::size_t NN, typename...> class Container,
            std::size_t N,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        static typename std::enable_if<
                  std::is_same<Key, common>::value
               && !details::has_insert<Container<C, N>>::value
            , Container<C, N>>::type get(A start, B end) {
            Container<C, N> container = {{ 0 }};

            for (std::size_t i = 0; i < N; ++i)
                container[i] = get<common>(start, end);

            return container;
        }

        /**
        * \brief Return random pointer from built-in array
        * \param array The built-in array with elements
        * \return Pointer to random element in array
        */
        template<typename T, std::size_t N>
        static T* get( T( &array )[ N ] ) {
            return std::addressof( array[ get<std::size_t>( 0, N - 1 ) ] );
        }

        /**
        * \brief Return value from custom Dist distribution
        *        seeded by internal random engine
        * \param Dist The type of custom distribution with next concept:
        *        http://en.cppreference.com/w/cpp/concept/RandomNumberDistribution
        * \param args The arguments which will be forwarded to Dist constructor
        * \return Value from custom distribution
        */
        template<typename Dist, typename... Args>
        static typename Dist::result_type get( Args&&... args ) {
            return Dist{ std::forward<Args>( args )... }( engine_instance( ) );
        }

        /**
        * \brief Return value from custom 'dist' distribution
        *        seeded by internal random engine
        * \param dist The custom distribution with next concept:
        *        http://en.cppreference.com/w/cpp/concept/RandomNumberDistribution
        * \param args The arguments which will be forwarded to Dist constructor
        * \return Value from custom 'dist' distribution
        */
        template<typename Dist>
        static typename Dist::result_type get( Dist& dist ) {
            return dist( engine_instance( ) );
        }

        /**
        * \brief Reorders the elements in the given range [first, last)
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param first, last - the range of elements to shuffle randomly       
        */
        template<typename RandomIt>
        static void shuffle( RandomIt first, RandomIt last ) {
            std::shuffle( first, last, engine_instance( ) );
        }

        /**
        * \brief Reorders the elements in the given container
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param container - the container with elements to shuffle randomly
        */
        template<typename Container>
        static void shuffle( Container& container ) {
            shuffle( std::begin( container ), std::end( container ) );
        }

        /// return internal engine by copy
        static Engine get_engine( ) {
            return engine_instance( );
        }

        /// return internal engine by ref
        static Engine& engine() {
            return engine_instance();
        }
    protected:
        /// get reference to the thread local engine instance
        static Engine& engine_instance( ) {
            thread_local Engine engine{ Seeder{ }( ) };
            return engine;
        }
    };

    /**
    * \brief Base template class for random 
    *        with local API and local internal member storage
    * \note it IS thread safe but less efficient then 
    *                           basic_random_static
    * \param Engine A random engine with interface like in the std::mt19937
    * \param Seeder A seeder type which return seed for internal engine 
    *                                             through operator()
    */
    template<
        typename Engine,
        typename Seeder = seeder_default,
        template<typename> class IntegerDist = std::uniform_int_distribution,
        template<typename> class RealDist = std::uniform_real_distribution,
        typename BoolDist = std::bernoulli_distribution
    >
    class basic_random_local {
    public:
        /// Type of used random number engine
        using engine_type = Engine;

        /// Type of used random number seeder
        using seeder_type = Seeder;

        /// Type of used integer distribution
        template<typename T>
        using integer_dist_t = IntegerDist<T>;

        /// Type of used real distribution
        template<typename T>
        using real_dist_t = RealDist<T>;

        /// Type of used bool distribution
        using bool_dist_t = BoolDist;

        /// Key type for getting common type numbers or objects
        using common = details::common;

        /**
        * \return The minimum value
        * potentially generated by the random-number engine
        */
        static constexpr typename Engine::result_type (min)( ) {
            return (Engine::min)( );
        }

        /**
        * \return The maximum value
        * potentially generated by the random-number engine
        */
        static constexpr typename Engine::result_type (max)( ) {
            return (Engine::max)( );
        }

        /// Advances the internal state by z times
        void discard( const unsigned long long z ) {
            m_engine.discard( z );
        }

        /// Reseed by Seeder
        void reseed( ) {
            Seeder seeder;
            seed( seeder( ) );
        }

        /**
        * \brief Reinitializes the internal state
        * of the random-number engine using new seed value
        * \param value The seed value to use 
        *        in the initialization of the internal state
        */
        void seed( const typename Engine::result_type value =
                          Engine::default_seed ) {
            m_engine.seed( value );
        }

        /**
        * \brief Reinitializes the internal state
        * of the random-number engine using new seed value
        * \param seq The seed sequence 
        *        to use in the initialization of the internal state
        */
        template<typename Sseq>
        void seed( Sseq& seq ) {
            m_engine.seed( seq );
        }

        /// return random number from engine in [min(), max()] range
        typename Engine::result_type get( ) {
            return m_engine( );
        }

        /**
        * \brief Compares internal pseudo-random number engine
        *        with 'other' pseudo-random number engine.
        *        Two engines are equal, if their internal states
        *        are equivalent, that is, if they would generate
        *        equivalent values for any number of calls of operator()
        * \param other The engine, with which the internal engine will be compared
        * \return true, if other and internal engine are equal
        */
        bool is_equal( const Engine& other ) {
            return m_engine == other;
        }

        /**
        * \brief Serializes the internal state of the
        *        internal pseudo-random number engine as a sequence
        *        of decimal numbers separated by one or more spaces,
        *        and inserts it to the stream ost. The fill character
        *        and the formatting flags of the stream are
        *        ignored and unaffected.
        * \param ost The output stream to insert the data to
        */
        template<typename CharT, typename Traits>
        void serialize( std::basic_ostream<CharT, Traits>& ost ) {
            ost << m_engine;
        }

        /**
        * \brief Restores the internal state of the
        *        internal pseudo-random number engine from
        *        the serialized representation, which
        *        was created by an earlier call to 'serialize'
        *        using a stream with the same imbued locale and
        *        the same CharT and Traits.
        *        If the input cannot be deserialized,
        *        internal engine is left unchanged and failbit is raised on ist
        * \param ost The input stream to extract the data from
        */
        template<typename CharT, typename Traits>
        void deserialize( std::basic_istream<CharT, Traits>& ist ) {
            ist >> m_engine;
        }

        /**
        * \brief Generate a random integer number in a [from; to] range
        *        by std::uniform_int_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random integer number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        typename std::enable_if<details::is_uniform_int<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            if( from < to ) // Allow range from higher to lower
                return IntegerDist<T>{ from, to }( m_engine );
            return IntegerDist<T>{ to, from }( m_engine );
        }

        /**
        * \brief Generate a random real number in a [from; to] range
        *        by std::uniform_real_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random real number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        typename std::enable_if<details::is_uniform_real<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            if( from < to ) // Allow range from higher to lower
                return RealDist<T>{ from, to }( m_engine );
            return RealDist<T>{ to, from }( m_engine );
        }

        /**
        * \brief Generate a random byte number in a [from; to] range
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random byte number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        typename std::enable_if<details::is_byte<T>::value
            , T>::type get( T from = (std::numeric_limits<T>::min)( ),
                            T to = (std::numeric_limits<T>::max)( ) ) {
            // Choose between short and unsigned short for byte conversion
            using short_t = typename std::conditional<std::is_signed<T>::value,
                short, unsigned short>::type;

            return static_cast<T>( get<short_t>( from, to ) );
        }

        /**
        * \brief Generate a random common_type number in a [from; to] range
        * \param Key The Key type for this version of 'get' method
        *        Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random common_type number in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Allow implicit type conversion
        * \note Prevent implicit type conversion from singed to unsigned types
        *       Why? std::common_type<Unsigned, Signed> chooses unsigned value,
        *                 then Signed value will be converted to Unsigned value
        *                       which gives us a wrong range for random values.
        *                           https://stackoverflow.com/a/5416498/5734836
        */
        template<
            typename Key,
            typename A,
            typename B, 
            typename C = typename std::common_type<A, B>::type
        >
        typename std::enable_if<
               std::is_same<Key, common>::value
            && details::is_supported_number<A>::value
            && details::is_supported_number<B>::value
            // Prevent implicit type conversion from singed to unsigned types
            && std::is_signed<A>::value != std::is_unsigned<B>::value
            , C>::type get( A from = (std::numeric_limits<A>::min)( ),
                            B to = (std::numeric_limits<B>::max)( ) ) {
            return get( static_cast<C>( from ), static_cast<C>( to ) );
        }

        /**
        * \brief Generate a random character in a [from; to] range
        *        by std::uniform_int_distribution
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \return A random character in a [from; to] range
        * \note Allow both: 'from' <= 'to' and 'from' >= 'to'
        * \note Prevent implicit type conversion
        */
        template<typename T>
        typename std::enable_if<details::is_supported_character<T>::value
            , T>::type get(T from = (std::numeric_limits<T>::min)(),
                T to = (std::numeric_limits<T>::max)()) {
            if (from < to) // Allow range from higher to lower
                return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(from), static_cast<std::int64_t>(to) }(m_engine));
            return static_cast<T>(IntegerDist<std::int64_t>{ static_cast<std::int64_t>(to), static_cast<std::int64_t>(from) }(m_engine));
        }

        /**
        * \brief Generate a bool value with specific probability
        *                         by std::bernoulli_distribution
        * \param probability The probability of generating true in [0; 1] range
        *        0 means always false, 1 means always true
        * \return 'true' with 'probability' probability ('false' otherwise)
        */
        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value
            , bool>::type get( const double probability = 0.5 ) {
            assert( 0 <= probability && 1 >= probability ); // out of [0; 1] range
            return BoolDist{ probability }( m_engine );
        }

        /**
        * \brief Return random value from initilizer_list
        * \param init_list initilizer_list with values
        * \return Random value from initilizer_list
        * \note Should be 1 or more elements in initilizer_list
        * \note Warning! Elements in initilizer_list can't be moved:
        *               https://stackoverflow.com/a/8193157/5734836
        */
        template<typename T>
        T get( std::initializer_list<T> init_list ) {
            assert( 0u != init_list.size( ) );
            return *get( init_list.begin( ), init_list.end( ) );
        }

        /**
        * \brief Return random iterator from iterator range
        * \param first, last - the range of elements
        * \return Random iterator from [first, last) range
        * \note If first == last, return last
        */
        template<typename InputIt>
        typename std::enable_if<details::is_iterator<InputIt>::value
            , InputIt>::type get( InputIt first, InputIt last ) {
            const auto size = std::distance( first, last );
            if( 0 == size ) return last;
            using diff_t = typename std::iterator_traits<InputIt>::difference_type;
            return std::next( first, get<diff_t>( 0, size - 1 ) );
        }

        /**
        * \brief Return random iterator from Container
        * \param container The container with elements
        * \return Random iterator from container
        * \note If container is empty return std::end( container ) iterator
        */
        template<typename Container>
        auto get( Container& container ) -> 
            typename std::enable_if<details::is_iterator<
                decltype(std::begin(container))>::value
                , decltype(std::begin(container))
            >::type {
            return get( std::begin( container ), std::end( container ) );
        }

        /**
        * \brief Return container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        * \note Container "reserve" method will be called before generation
        */
        template<template<typename...> class Container, typename A>
        typename std::enable_if<
                details::has_reserve<Container<A>>::value
            , Container<A>>::type get(A from, A to, std::size_t size) {
            Container<A> container;

            container.reserve(size);
            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get(from, to));

            return container;
        }

        /**
        * \brief Return container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        * \note Container "reserve" method will be called before generation
        */
        template<
            template<typename...> class Container,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        typename std::enable_if<
                   std::is_same<Key, common>::value
                && details::has_reserve<Container<A>>::value
            , Container<C>>::type get(A start, B end, std::size_t size) {
            Container<C> container;

            container.reserve(size);
            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get<common>(start, end));

            return container;
        }

        /**
        * \brief Return container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<template<typename...> class Container, typename A>
        typename std::enable_if<
              !details::has_reserve<Container<A>>::value
            , Container<A>>::type get(A start, A end, std::size_t size) {
            Container<A> container;

            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get(start, end));

            return container;
        }

        /**
        * \brief Return container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<
            template<typename...> class Container,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        typename std::enable_if<
                 std::is_same<Key, common>::value
              && !details::has_reserve<Container<C>>::value
            , Container<C>>::type get(A start, B end, std::size_t size) {
            Container<C> container;

            for (std::size_t i = 0; i < size; ++i)
                container.insert(std::end(container), get<common>(start, end));

            return container;
        }

        /**
        * \brief Return array-like container filled with random values
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param N The number of elements in resulting container
        * \return Container filled with random values
        */
        template<template<typename AA, std::size_t NN, typename...> class Container, std::size_t N, typename A>
        typename std::enable_if<
               !details::has_insert<Container<A, N>>::value
            , Container<A, N>>::type get(A start, A end) {
            Container<A, N> container = {{ 0 }};

            for (std::size_t i = 0; i < N; ++i)
                container[i] = get(start, end);

            return container;
        }

        /**
        * \brief Return array-like container filled with random common_type values
        * \param Key The Key type for this version of 'get' method
        *     Type should be '(THIS_TYPE)::common' struct
        * \param from The first limit number of a random range
        * \param to The second limit number of a random range
        * \param size The number of elements in resulting container
        * \return Container filled with random values
        */
        template<
            template<typename AA, std::size_t NN, typename...> class Container,
            std::size_t N,
            typename Key,
            typename A,
            typename B,
            typename C = typename std::common_type<A, B>::type>
        typename std::enable_if<
                  std::is_same<Key, common>::value
               && !details::has_insert<Container<C, N>>::value
            , Container<C, N>>::type get(A start, B end) {
            Container<C, N> container = {{ 0 }};

            for (std::size_t i = 0; i < N; ++i)
                container[i] = get<common>(start, end);

            return container;
        }

        /**
        * \brief Return random pointer from built-in array
        * \param array The built-in array with elements
        * \return Pointer to random element in array
        */
        template<typename T, std::size_t N>
        T* get( T( &array )[ N ] ) {
            return std::addressof( array[ get<std::size_t>( 0, N - 1 ) ] );
        }

        /**
        * \brief Return value from custom Dist distribution
        *        seeded by internal random engine
        * \param Dist The type of custom distribution with next concept:
        *        http://en.cppreference.com/w/cpp/concept/RandomNumberDistribution
        * \param args The arguments which will be forwarded to Dist constructor
        * \return Value from custom distribution
        */
        template<typename Dist, typename... Args>
        typename Dist::result_type get( Args&&... args ) {
            return Dist{ std::forward<Args>( args )... }( m_engine );
        }

        /**
        * \brief Return value from custom 'dist' distribution
        *        seeded by internal random engine
        * \param dist The custom distribution with next concept:
        *        http://en.cppreference.com/w/cpp/concept/RandomNumberDistribution
        * \param args The arguments which will be forwarded to Dist constructor
        * \return Value from custom 'dist' distribution
        */
        template<typename Dist>
        typename Dist::result_type get( Dist& dist ) {
            return dist( m_engine );
        }

        /**
        * \brief Reorders the elements in the given range [first, last)
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param first, last - the range of elements to shuffle randomly       
        */
        template<typename RandomIt>
        void shuffle( RandomIt first, RandomIt last ) {
            std::shuffle( first, last, m_engine );
        }

        /**
        * \brief Reorders the elements in the given container
        *        such that each possible permutation of those elements
        *        has equal probability of appearance.
        * \param container - the container with elements to shuffle randomly
        */
        template<typename Container>
        void shuffle( Container& container ) {
            shuffle( std::begin( container ), std::end( container ) );
        }

        /// return internal engine by copy
        Engine get_engine( ) const {
            return m_engine;
        }

        /// return internal engine by ref
        Engine& engine() {
            return m_engine;
        }
    protected:
        /// return engine seeded by Seeder
        static Engine make_seeded_engine( ) {
            // Make seeder instance for seed return by reference like std::seed_seq
            return Engine{ Seeder{ }( ) };
        }
    protected:
        /// The random number engine
        Engine m_engine{ make_seeded_engine( ) };
    };

    /** 
    * \brief The basic static random alias based on a std::mt19937
    * \note It uses static methods API and data with static storage
    * \note Not thread safe but more prefomance
    */
    using random_static = basic_random_static<std::mt19937>;

    /**
    * \brief The basic static random alias based on a std::mt19937
    * \note It uses static methods API and data with thread_local storage
    * \note Thread safe but less performance
    */
    using random_thread_local = basic_random_thread_local<std::mt19937>;

    /**
    * \brief The basic static random alias based on a std::mt19937
    * \note It uses non static methods API and data with auto storage
    * \note Not thread safe. Should construct on the stack at local scope
    */
    using random_local = basic_random_local<std::mt19937>;

} // namespace effolkronium

#endif // #ifndef EFFOLKRONIUM_RANDOM_HPP
