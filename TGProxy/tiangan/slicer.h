/*
 * slicer.h
 *
 * Created on: 2017年11月16日
 * Author: zhujun
 * package tiangan-ps-v1
 * @Weibo @半饱半醉
 */

#ifndef INCLUDE_TIANGAN_SLICER_H_
#define INCLUDE_TIANGAN_SLICER_H_

#include <string>
#include <limits>
#include <vector>
#include "tiangan.h"

namespace tiangan {

    template<class K>
    class KeyRange {
    public:
        KeyRange() : KeyRange(0, 0) {}
        KeyRange(K begin, K end) : begin_(begin), end_(end) {}

        K begin() const { return begin_; }
        K end() const { return end_; }
        K size() const { return end_ - begin_; }

    private:
        K begin_;
        K end_;
    };

    template<class K>
    class RangeSlicer {
    public:
        typedef std::vector<K> Slice;

        RangeSlicer(int range) {
            K maxval = std::numeric_limits<K>::max();
            for (int i = 0; i < range; ++i) {
                ranges_.push_back(KeyRange<K>(maxval / range * i, maxval / range * (i+1)));
            }
        }

        ~RangeSlicer() {
        }

        template<class InputIt, class PositionIt>
        void slice(InputIt begin, InputIt end, PositionIt pos) {
            size_t n = ranges_.size();

            for (size_t i = 0; i < n; ++i) {
                if (i == 0) {
                    pos[0] = std::lower_bound(begin, end, ranges_[0].begin()) - begin;
                    begin += pos[0];
                }
                size_t len = std::lower_bound(begin, end, ranges_[i].end()) - begin;
                begin += len;
                pos[i+1] = pos[i] + len;
            }
        }

        void slice(const std::vector<K>& keys, std::vector<Slice>& slices) {
            slices.resize(ranges_.size());
            size_t n = ranges_.size();
            std::vector<size_t> pos(n+1);
            auto begin = keys.begin();
            auto end = keys.end();

            slice(begin, end, pos.begin());
            if (keys.empty())
                return;

            for (size_t i = 0; i < n; ++i) {
                if (pos[i+1] == pos[i]) {
                    continue;
                }
                slices[i].resize(pos[i+1] - pos[i]);
                auto begin = keys.begin();
                copy(begin + pos[i], begin + pos[i + 1], slices[i].begin());
            }
        }

        const std::vector<KeyRange<K>>& range() const {
            return ranges_;
        }

    protected:
        std::vector<KeyRange<K>> ranges_;
    };

}

#endif /* INCLUDE_TIANGAN_SLICER_H_ */
