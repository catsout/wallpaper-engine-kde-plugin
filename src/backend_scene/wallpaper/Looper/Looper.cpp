#include "Looper.hpp"
#include <algorithm>

#include "Utils/Visitors.hpp"
#include "Utils/Logging.h"

#include <cassert>

using namespace wallpaper::looper;

using Lock = std::unique_lock<std::mutex>;

void Looper::setName(std::string_view name) {
    m_name.assign(name);
}

const std::string_view  Looper::name() const {
    return m_name;
}

Looper::Looper() {}
Looper::~Looper() {
    stop();
}

bool Looper::loop() {
    MessageWrapper msg;
    {
        Lock lock(m_mutex);
        if(!m_running)
            return false;
        if(m_msg_queue.empty()) {
            m_condition.wait(lock);
            return true;
        }
        msg = *m_msg_queue.begin();
        m_msg_queue.erase(m_msg_queue.begin());
    }
    msg.msg->deliver();
    if(msg.msg->cleanAfterDeliver()) {
        msg.msg->cleanContent();
    }
    return true;
}

status_t Looper::start() {
	Lock lock(m_mutex);
    if(m_running) return status_t::INVALID_OPERATION;
    // using weak_ptr to allow looper deleted at looper->loop() end
    std::weak_ptr<Looper> wlooper = shared_from_this();
    m_thread = std::thread([](std::weak_ptr<Looper> wlooper) {
        Looper* looper = nullptr;
        {
            looper = wlooper.lock().get();
            LOG_INFO("%s looper started", looper->name().data());
            looper->m_running = true;
        }
        std::string name {looper->name()};
        while(!wlooper.expired() && looper->m_running && looper->loop()) {
        }
        LOG_INFO("%s looper stopped", name.c_str());
    }, wlooper);
    return status_t::OK;
}

void Looper::stop() {
    if(!m_running) return;

    std::thread thd;
    {
        Lock lock(m_mutex);
        m_thread.swap(thd);
        m_running = false;
        m_condition.notify_one();
    }
   	if(thd.joinable()) {
        if(std::this_thread::get_id() == thd.get_id()) {
            LOG_INFO("detach %s looper", m_name.c_str());
            thd.detach();
        }
        else {
		    thd.join();
        }
	}
}


void Looper::post(const std::shared_ptr<Message>& msg) {
    {
        Lock lock(m_mutex);
        m_msg_queue.insert(m_msg_queue.end(), {msg});
    }
    m_condition.notify_one();
}

handler_id Looper::registerHandler(const std::shared_ptr<Handler>& handler) {
    static std::atomic<handler_id> next_handle_id {1};
    if(handler->setID(next_handle_id++, shared_from_this())) {
        Lock lock(m_mutex);
        m_reg_handler[handler->id()] = handler;
        return handler->id();
    }
    return Handler::INVALID_HANDLER_ID;
}

void Looper::unregisterHandler(handler_id id) {
    Lock lock(m_mutex);
    if(m_reg_handler.count(id) > 0) {
        auto handle = m_reg_handler[id].lock();
        if(handle != nullptr) {
            handle->setID(id, {});
        }
        m_reg_handler.erase(id);
    }
}

Handler::Handler()
    : m_id(INVALID_HANDLER_ID){
}

int32_t Handler::id() const { return m_id; }
std::weak_ptr<Looper> Handler::getLooper() const { return m_looper; };

bool Handler::setID(handler_id id, std::weak_ptr<Looper> looper) {
    handler_id iv = INVALID_HANDLER_ID;
    if(m_id.compare_exchange_weak(iv, id)) {
        m_looper = looper;
        return true;
    }
    return false;
}

void Handler::deliverMessage(const std::shared_ptr<Message>& msg) {
    onMessageReceived(msg);
}


Message::Message():
    m_what(0),
    m_target(Handler::INVALID_HANDLER_ID) {}
Message::Message(uint32_t what, const std::shared_ptr<Handler>& handler):
    m_what(what) {
    setTarget(handler);
}

uint32_t Message::what() const { return m_what; };

std::shared_ptr<Message> Message::create() {
    struct Message_sp : public Message {};
    return std::make_shared<Message_sp>();
}
std::shared_ptr<Message> Message::create(uint32_t what, const std::shared_ptr<Handler>& handler) {
    return std::shared_ptr<Message>(new Message(what, handler));
}

void Message::setTarget(const std::shared_ptr<Handler>& handler) {
    if(handler == nullptr) {
        m_target = Handler::INVALID_HANDLER_ID;
        m_handler.reset();
        m_looper.reset();
    } else {
        m_target = handler->id();
        m_handler = handler; // weak
        m_looper = handler->getLooper(); //weak
    }
}

void Message::setWhat(uint32_t what) {
    m_what = what;
}

status_t Message::post() {
    auto looper = m_looper.lock();
    if(looper != nullptr) {
        looper->post(shared_from_this());
        return status_t::OK;
    }
    return status_t::NOT_FOUND;
}

void Message::deliver() {
    auto handler = m_handler.lock();
    if(handler != nullptr) {
        handler->deliverMessage(shared_from_this());
    }
}

void Message::Item::setName(std::string_view name) {
    this->name.assign(name);
}

size_t Message::countEntries() const {
    return m_num_items;
}

const Message::Item* Message::getEntryAt(int32_t index) const {
    if(index >= m_num_items || index >= MaxNumItems) return nullptr;
    return &m_items[index];
}

template<typename T>
const Message::Item* Message::findItem(std::string_view name) const {
    auto end = m_items.begin() + m_num_items;
    auto it = std::find_if(m_items.begin(), end, [name](const Item& item) {
        bool ok = item.name == name &&
            std::holds_alternative<T>(item.value);
        return ok;
    });
    return it != end ? &*it : nullptr;
}

Message::Item* Message::allocateItem(std::string_view name) {
    Item* item {nullptr};
    if(m_num_items >= MaxNumItems) 
        return item;
    auto end = m_items.begin() + m_num_items;
    auto it = std::find_if(m_items.begin(), end, [name](const Item& item) {
        return item.name == name;
    });
    item = &*it;
    if(it == end) {
        item->name.assign(name);
        m_num_items++;
    }

    return item;
}

#define BASIC_TYPE(NAME,TYPENAME)                                        \
bool Message::set##NAME(std::string_view name, TYPENAME value) {         \
    Item *item = allocateItem(name);                                     \
    if(item == nullptr) return false;                                    \
                                                                         \
    item->value = value;                                                 \
    return true;                                                         \
}                                                                        \
                                                                         \
bool Message::find##NAME(std::string_view name, TYPENAME *value) const { \
    const Item *item = findItem<TYPENAME>(name);                         \
    if (item) {                                                          \
        auto* iv = std::get_if<TYPENAME>(&(item->value));                \
        if(iv == nullptr) return false;                                  \
        *value = *iv;                                                    \
        return true;                                                     \
    }                                                                    \
    return false;                                                        \
}

BASIC_TYPE(Bool, bool)
BASIC_TYPE(Int32, int32_t)
BASIC_TYPE(Float, float)
BASIC_TYPE(String, std::string)

bool Message::setObject(std::string_view name, const std::shared_ptr<void>& value) {
    Item *item = allocateItem(name);
    if(item == nullptr) return false;
    item->value = value;
    return true;
}

template
const Message::Item* Message::findItem<std::shared_ptr<void>>(std::string_view) const;

bool Message::cleanAfterDeliver() const { return m_clean_after_dliver; }
void Message::setCleanAfterDeliver(bool v) { m_clean_after_dliver = v; };
void Message::cleanContent() {
    m_items.fill({});
}