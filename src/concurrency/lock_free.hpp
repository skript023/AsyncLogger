#pragma once
#include <atomic>
#include <memory>

namespace al
{
    template<typename T>
    class lock_free_queue
    {
    private:
        struct node
        {
            std::shared_ptr<T> data;
            std::atomic<node*> next{nullptr};
            node() = default;
            explicit node(T value) : data(std::make_shared<T>(std::move(value))) {}
        };

        std::atomic<node*> head;
        std::atomic<node*> tail;

    public:
        lock_free_queue()
        {
            node* dummy = new node(); // dummy node untuk inisialisasi
            head.store(dummy);
            tail.store(dummy);
        }

        ~lock_free_queue()
        {
            while (auto* old_head = head.load())
            {
                head.store(old_head->next);
                delete old_head;
            }
        }

        void push(T value)
        {
            auto new_node = new node(std::move(value));
            node* old_tail = nullptr;

            // CAS loop
            while (true)
            {
                old_tail = tail.load(std::memory_order_acquire);
                node* next = old_tail->next.load(std::memory_order_acquire);

                if (next == nullptr)
                {
                    if (old_tail->next.compare_exchange_weak(next, new_node))
                        break; // success
                }
                else
                {
                    // tail ketinggalan — bantu geser ke depan
                    tail.compare_exchange_weak(old_tail, next);
                }
            }

            // coba geser tail ke node baru
            tail.compare_exchange_weak(old_tail, new_node);
        }

        bool try_pop(T& result)
        {
            node* old_head = nullptr;

            while (true)
            {
                old_head = head.load(std::memory_order_acquire);
                node* next = old_head->next.load(std::memory_order_acquire);

                if (next == nullptr)
                {
                    return false; // queue kosong
                }

                if (head.compare_exchange_weak(old_head, next))
                {
                    result = std::move(*next->data);
                    delete old_head; // hapus node dummy lama
                    return true;
                }
            }
        }

        void wait_and_pop(T& result)
        {
            size_t spin_count = 0;
            while (!try_pop(result))
            {
                // exponential backoff
                if (spin_count < 10)
                {
                    std::this_thread::yield();
                }
                else if (spin_count < 100)
                {
                    _mm_pause(); // on x86 — reduces power & contention
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(50));
                }
                ++spin_count;
            }
        }

        bool empty() const
        {
            node* head_ptr = head.load(std::memory_order_acquire);
            node* next = head_ptr->next.load(std::memory_order_acquire);
            return (next == nullptr);
        }
    };
}
