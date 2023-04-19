#include "SmartLeds.h"


/* PlatformIO 6 (ESP IDF 5) does no longer allow access to RMTMEM,
   see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.0/peripherals.html?highlight=rmtmem#id5 
   As a dirty workaround, we copy the needed structures from rmt_struct.h
   In the long run, this should be replaced! */
typedef struct rmt_item32_s {
    union {
        struct {
            uint32_t duration0 :15;
            uint32_t level0 :1;
            uint32_t duration1 :15;
            uint32_t level1 :1;
        };
        uint32_t val;
    };
} rmt_item32_t;

//Allow access to RMT memory using RMTMEM.chan[0].data32[8]
typedef volatile struct rmt_mem_s {
    struct {
        rmt_item32_t data32[64];
    } chan[8];
} rmt_mem_t;
extern rmt_mem_t RMTMEM;



IsrCore SmartLed::_interruptCore = CoreCurrent;
intr_handle_t SmartLed::_interruptHandle = NULL;

SmartLed*& IRAM_ATTR SmartLed::ledForChannel( int channel ) {
    static SmartLed* table[8] = { nullptr };
    assert( channel < 8 );
    return table[ channel ];
}

void IRAM_ATTR SmartLed::interruptHandler(void*) {
    for (int channel = 0; channel != 8; channel++) {
        auto self = ledForChannel( channel );

        if ( RMT.int_st.val & (1 << (24 + channel ) ) ) { // tx_thr_event
            if ( self )
                self->copyRmtHalfBlock();
            RMT.int_clr.val |= 1 << ( 24 + channel );
        } else if ( RMT.int_st.val & ( 1 << (3 * channel ) ) ) { // tx_end
            if ( self )
                xSemaphoreGiveFromISR( self->_finishedFlag, nullptr );
            RMT.int_clr.val |= 1 << ( 3 * channel );
        }
    }
}

void IRAM_ATTR SmartLed::copyRmtHalfBlock() {
    int offset = detail::MAX_PULSES * _halfIdx;
    _halfIdx = !_halfIdx;
    int len = 3 - _componentPosition + 3 * ( _count - 1 );
    len = std::min( len, detail::MAX_PULSES / 8 );

    if ( !len ) {
        for ( int i = 0; i < detail::MAX_PULSES; i++) {
            RMTMEM.chan[ _channel].data32[i + offset ].val = 0;
        }
    }

    int i;
    for ( i = 0; i != len && _pixelPosition != _count; i++ ) {
        uint8_t val = _buffer[ _pixelPosition ].getGrb( _componentPosition );
        for ( int j = 0; j != 8; j++, val <<= 1 ) {
            int bit = val >> 7;
            int idx = i * 8 + offset + j;
            RMTMEM.chan[ _channel ].data32[ idx ].val = _bitToRmt[ bit & 0x01 ].value;
        }
        if ( _pixelPosition == _count - 1 && _componentPosition == 2 ) {
            RMTMEM.chan[ _channel ].data32[ i * 8 + offset + 7 ].duration1 =
                _timing.TRS / ( detail::RMT_DURATION_NS * detail::DIVIDER );
        }

        _componentPosition++;
        if ( _componentPosition == 3 ) {
            _componentPosition = 0;
            _pixelPosition++;
        }
    }

    for ( i *= 8; i != detail::MAX_PULSES; i++ ) {
        RMTMEM.chan[ _channel ].data32[ i + offset ].val = 0;
    }
}
