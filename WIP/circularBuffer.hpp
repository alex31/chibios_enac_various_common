#pragma once
#include "ch.h"
#include "stdutil.h"
#include <array>
#include <cstddef>
#include <cstring>
#include <optional>
#include <span>
#include <etl/vector.h> // Assumed include for ETL vector

// Lightweight circular buffer without dynamic allocation, no exceptions.
// Templated on element type T, capacity S, and optional MutexPolicy (stubbed for now).

template <typename T, std::size_t S, typename MutexPolicy = void>
class CircularBuffer {
public:
    using value_type = T;
    using size_type = std::size_t;
    static constexpr size_type capacity = S;
    static constexpr size_type max_view_spans = 2;
    using Span = std::span<T>;
    using SpanList = etl::vector<Span, max_view_spans>;

    CircularBuffer() noexcept
        : head_(0), tail_(0), count_(0), last_view_count_(0)
    {}

    // Write a single element. Returns 1 on success, 0 if full.
    size_type write(const T& item) noexcept {
        if (count_ >= capacity) {
            return 0;
        }
        buffer_[tail_] = item;
        tail_ = increment(tail_);
        ++count_;
        return 1;
    }

  // Write multiple elements. Returns number of elements written (0 if no space).
  // Dans votre méthode write(std::span<const T> items)
  size_type write(std::span<const T> items) noexcept {
    size_type available = capacity - count_;
    size_type to_write = (items.size() < available ? items.size() : available);
    if constexpr (std::is_trivially_copyable_v<T>) {
      // cas trivially copyable : on fait 1 ou 2 memcpy
      size_type first_chunk = std::min(to_write, capacity - tail_);
      std::memcpy(&buffer_[tail_], items.data(), first_chunk * sizeof(T));
      tail_ = (tail_ + first_chunk) % capacity;
      if (first_chunk < to_write) {
	size_type second_chunk = to_write - first_chunk;
	std::memcpy(&buffer_[0], items.data() + first_chunk, second_chunk * sizeof(T));
	tail_ = second_chunk;
      }
      count_ += to_write;
    } else {
        // cas générique : copie élément par élément
      for (size_type i = 0; i < to_write; ++i) {
	buffer_[tail_] = items[i];
	tail_ = increment(tail_);
	++count_;
      }
    }
    return to_write;
  }
  
    // Read one element (by copy). Returns std::optional<T>.
    std::optional<T> read() noexcept {
        if (count_ == 0) {
            return std::nullopt;
        }
        T val = buffer_[head_];
        head_ = increment(head_);
        --count_;
        return val;
    }

    // Peek up to n elements without removing. Implicitly consumes any previous view.
    SpanList view(size_type n) noexcept {
        if (last_view_count_ > 0) {
            consume();
        }
        SpanList spans;
        size_type to_view = (n < count_ ? n : count_);
        if (to_view == 0) {
            last_view_count_ = 0;
            return spans;
        }
        // Single contiguous block
        if (head_ + to_view <= capacity) {
            spans.emplace_back(&buffer_[head_], to_view);
        } else {
            // Wrap-around: two spans
            size_type first_len = capacity - head_;
            spans.emplace_back(&buffer_[head_], first_len);
            size_type second_len = to_view - first_len;
            spans.emplace_back(&buffer_[0], second_len);
        }
        last_view_count_ = to_view;
        return spans;
    }

    // Consume elements previously viewed.
    void consume() noexcept {
        if (last_view_count_ > 0) {
            head_ = (head_ + last_view_count_) % capacity;
            count_ -= last_view_count_;
            last_view_count_ = 0;
        }
    }

    // Utility
    bool empty() const noexcept { return count_ == 0; }
    bool full() const noexcept { return count_ >= capacity; }
    size_type size() const noexcept { return count_; }

private:
    std::array<T, capacity> buffer_;
    size_type head_;
    size_type tail_;
    size_type count_;
    size_type last_view_count_;

    // Increment index modulo capacity
    static constexpr size_type increment(size_type idx) noexcept {
        return (idx + 1) < capacity ? (idx + 1) : 0;
    }
};

#include <cassert>
#include <iostream>
#include <vector>

void test_circular_buffer()
{
    // On choisit un buffer de capacité 5 pour les tests
    constexpr size_t CAP = 5;
    CircularBuffer<int, CAP> buf;

    // 1) À l'initialisation : vide, pas plein, taille 0
    chDbgAssert(buf.empty(), "Initial buffer should be empty");
    chDbgAssert(!buf.full(), "Initial buffer should not be full");
    chDbgAssert(buf.size() == 0, "Initial size should be 0");

    // 2) write(const T&)
    for (int i = 1; i <= 5; ++i) {
        size_t written = buf.write(i);
        chDbgAssert(written == 1, "write(int) should write exactly one element");
        chDbgAssert(buf.size() == static_cast<size_t>(i), "Size should increment with each write");
    }
    // buffer doit être plein et refuser toute écriture
    chDbgAssert(buf.full(), "Buffer should be full after 5 writes");
    chDbgAssert(buf.write(42) == 0, "Write on full buffer should return 0");
    chDbgAssert(buf.size() == CAP, "Size should remain CAP when write fails");

    // 3) read() jusqu'à vider
    for (int expect = 1; expect <= 5; ++expect) {
        auto opt = buf.read();
        chDbgAssert(opt.has_value(), "read() should return a value when buffer is not empty");
        chDbgAssert(opt.value() == expect, "read() value does not match expected sequence");
    }
    chDbgAssert(!buf.read().has_value(), "read() on empty buffer should return no value");
    chDbgAssert(buf.empty(), "Buffer should be empty after reads");
    chDbgAssert(!buf.full(), "Buffer should not be full after reads");
    chDbgAssert(buf.size() == 0, "Size should be 0 after reads");

    // 4) write(span)
    std::vector<int> v = {10, 11, 12, 13, 14, 15};
    size_t w = buf.write(std::span(v));
    chDbgAssert(w == CAP, "write(span) should write up to capacity");
    chDbgAssert(buf.full(), "Buffer should be full after write(span)");
    chDbgAssert(buf.size() == CAP, "Size should be CAP after write(span)");

    // 5) view(n) + consume() sans wrap-around
    {
        auto spans = buf.view(3);
        chDbgAssert(spans.size() == 1, "view(3) should return one span when no wrap-around");
        chDbgAssert(spans[0].size() == 3, "Span length should be equal to requested count");
        chDbgAssert(spans[0][0] == 10 && spans[0][1] == 11 && spans[0][2] == 12, "Span contents mismatch");
        chDbgAssert(buf.size() == CAP, "Size should not change before consume()");
        buf.consume();
        chDbgAssert(buf.size() == CAP - 3, "Size should decrease by view count after consume()");
    }

    // 6) test wrap-around pour view
    buf = {}; // remettre à zéro
    buf.write(std::span(v));         // écrit CAP éléments
    buf.read(); buf.read();          // lit 2 => size=3
    std::vector<int> v3 = {20,21,22,23};
    w = buf.write(std::span(v3));    // doit écrire min(4, CAP-3)=2
    chDbgAssert(w == 2, "write(span) should write only available space");
    chDbgAssert(buf.size() == CAP, "Buffer should be full after wrap-around writes");
    {
        auto spans = buf.view(5);
        chDbgAssert(spans.size() == 2, "view on wrapped buffer should return two spans");
        chDbgAssert(spans[0].size() == 3 && spans[1].size() == 2, "Span sizes incorrect on wrap-around");
        chDbgAssert(spans[0][0] == 12 && spans[0][1] == 13 && spans[0][2] == 14, "First span contents mismatch");
        chDbgAssert(spans[1][0] == 20 && spans[1][1] == 21, "Second span contents mismatch");
        buf.consume();
        chDbgAssert(buf.empty(), "Buffer should be empty after consume() on wrapped view");
    }

    // 7) read après consume()
    chDbgAssert(!buf.read().has_value(), "read() on empty buffer after consume() should return no value");

    // Remplace l'affichage sur stdout par DebugTrace
    DebugTrace("Tous les tests de CircularBuffer ont réussi 🎉\n");
}

