#include "region_data_provider.h"

#include <google/protobuf/wire_format_lite.h>

namespace NInPlaceProto {

    ui32 TRegionDataProvider::ReadVarint32Slow(ui32 b) noexcept {
        ui32 res = 0;
        do {
            res |= b & 0x7F;
            if (Y_LIKELY(b < 0x80)) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            b = *Start++;
            res |= static_cast<ui32>(b & 0x7F) << 7;
            if (b < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            b = *Start++;
            res |= static_cast<ui32>(b & 0x7F) << 14;
            if (b < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            b = *Start++;
            res |= static_cast<ui32>(b & 0x7F) << 21;
            if (b < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            b = *Start++;
            res |= static_cast<ui32>(b & 0x7F) << 28;
            if (b < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            // 5 more bytes
            if (*Start++ < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            if (*Start++ < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            if (*Start++ < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            if (*Start++ < 0x80) {
                break;
            }
            if (Y_UNLIKELY(Start >= End)) {
                Corrupted = true;
                return 0;
            }
            if (*Start++ < 0x80) {
                break;
            }
            Corrupted = true;
            return 0;
        } while (false);
        return res;
    }
    ui64 TRegionDataProvider::ReadVarint64Slow() noexcept {
        ui64 result = 0;
        int count = 0;
        ui32 b;
        const ui8* start = Start;
        const ui8*const end = End;

        do {
            if (count == 10 || start >= end) {
                Corrupted = true;
                Start = start;
                return 0;
            }
            b = *start++;
            result |= static_cast<ui64>(b & 0x7F) << (7 * count++);
        } while (b >= 0x80);

        Start = start;
        return result;
    }

} // namespace NInPlaceProto
