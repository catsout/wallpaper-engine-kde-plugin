#pragma once
#include <memory>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>
#include <list>
#include <map>
#include <string_view>
#include <variant>

#include "Utils/Visitors.hpp"
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{
namespace looper
{

enum class status_t {
    OK                = 0,
    INVALID_OPERATION = -ENOSYS,
    NOT_FOUND         = -ENOENT,
    NO_MEM            = -ENOMEM,
    BUSY              = -EBUSY
};
using handler_id = int32_t;
class Handler;
class Message;

class Looper : NoCopy,public std::enable_shared_from_this<Looper> {
public:
    Looper();
    ~Looper();

    handler_id registerHandler(const std::shared_ptr<Handler>&);
    void unregisterHandler(handler_id id);
    status_t start();
    void stop();
    void post(const std::shared_ptr<Message>&);
    const std::string_view name() const;
    void setName(std::string_view);
private:
    struct MessageWrapper {
        std::shared_ptr<Message> msg;
    };
    bool loop();

    bool m_running {false};
    std::string m_name {"unknown"};
    std::mutex m_mutex;
    std::condition_variable m_condition;

    std::thread m_thread;
    std::list<MessageWrapper> m_msg_queue;
    std::map<handler_id, std::weak_ptr<Handler>> m_reg_handler;
};

class Handler : NoCopy,public std::enable_shared_from_this<Handler> {
public:
    const static handler_id INVALID_HANDLER_ID = 0;
    Handler();
    virtual ~Handler() = default;

    handler_id id() const;
    std::weak_ptr<Looper> getLooper() const;

protected:
    virtual void onMessageReceived(const std::shared_ptr<Message>&) = 0;
private:
    friend class Looper;
    friend class Message;

    // compare and set
    bool setID(handler_id id, std::weak_ptr<Looper> looper);

    void deliverMessage(const std::shared_ptr<Message>& msg);

    std::atomic<handler_id> m_id;
    std::weak_ptr<Looper> m_looper;
};

class Message : public std::enable_shared_from_this<Message> {
private:
    Message();
    Message(uint32_t what, const std::shared_ptr<Handler>&);
    friend class Looper;

public:
    static std::shared_ptr<Message> create();
    static std::shared_ptr<Message> create(uint32_t what, const std::shared_ptr<Handler>&);
    uint32_t what() const;
    void setTarget(const std::shared_ptr<Handler>&);
    void setWhat(uint32_t);
    status_t post();
    //status_t postAndWaitResponse(const std::shared_ptr<Message>&);

private:
    std::weak_ptr<Handler> m_handler;
    std::weak_ptr<Looper> m_looper;

    uint32_t m_what;
    handler_id m_target;

    void deliver();

public:

    using ItemValue = std::variant<
            bool,
            int32_t,
            float,
            std::string,
            std::shared_ptr<void>,
            visitor::NoType>;

    struct Item {
        ItemValue   value {visitor::NoType()};
        std::string name; 
        void setName(std::string_view name);
    };

    size_t countEntries() const;
    const Item* getEntryAt(int32_t index) const;


    bool setBool(std::string_view, bool);
    bool setInt32(std::string_view, int32_t);
    bool setFloat(std::string_view, float);
    bool setString(std::string_view, std::string);

    bool findBool(std::string_view, bool *) const;
    bool findInt32(std::string_view, int32_t *) const;
    bool findFloat(std::string_view, float *) const;
    bool findString(std::string_view, std::string *) const;

    bool setObject(std::string_view name, const std::shared_ptr<void>&);

    template <typename T>
    bool findObject(std::string_view name, std::shared_ptr<T>* value) const {
        const Item *item = findItem<std::shared_ptr<void>>(name);
        if (item) {
            auto* iv = std::get_if<std::shared_ptr<void>>(&(item->value));
            if(iv == nullptr) return false;
            *value = *(reinterpret_cast<const std::shared_ptr<T>*>(iv));
            return true;
        }
        return false; 
    }

private:
    template<typename T>
    const Item* findItem(std::string_view name) const;
    Item* allocateItem(std::string_view name);

    constexpr static int32_t MaxNumItems = 64;
    std::array<Item, MaxNumItems> m_items;
    size_t m_num_items {0};
};

}
}
