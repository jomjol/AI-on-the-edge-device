#include "SmartLeds.h"

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
