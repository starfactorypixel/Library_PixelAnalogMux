#pragma once
#include <inttypes.h>
#include <DrakePinD.hpp>

template <uint8_t _count = 0, uint16_t _tick_time = 25> 
class AnalogMux
{
	using func_adc_req_t = uint16_t (*)(uint8_t address);
	using func_result_t = void (*)(uint8_t address, uint16_t value);
	
	public:
		
		template <typename... DPinTs> 
		AnalogMux(func_adc_req_t adc_req, func_result_t result, DPinTs... dpins) : 
			_callback_adc_req(adc_req), 
			_callback_result(result),
			_digital{ DrakePinD(dpins, DrakePin::Output, DrakePin::Low)... },
			_adc_value{}
		{
			static_assert(sizeof...(dpins) == _count, "Incorrect number of digital pins provided");
		}

		void Init()
		{
			for(DrakePinD &pin : _digital)
			{
				pin.Init();
			}
			
			return;
		}
		
		uint16_t Get(uint8_t address)
		{
			_SetPins(address);
			
			return _callback_adc_req(address);
		}

		void Processing(uint32_t &time)
		{
			if(time - _last_tick < _tick_time) return;
			_last_tick = time;
			
			for(uint8_t addr = 0; addr < (1 << _count); ++addr)
			{
				_adc_value[addr] = Get(addr);
				_callback_result(addr, _adc_value[addr]);
			}
			
			return;
		}

		const uint16_t *adc_value = _adc_value;

	private:

		void _SetPins(uint8_t address)
		{
			for(uint8_t i = 0; i < _count; ++i)
			{
				if(address & (1 << i)){
					_digital[i].On();
				} else {
					_digital[i].Off();
				}
			}
			asm("nop; nop; nop; \n");
			
			return;
		}
		
		func_adc_req_t _callback_adc_req = nullptr;
		func_result_t _callback_result = nullptr;
		
		DrakePinD _digital[_count];
		uint16_t _adc_value[(1 << _count)];
		
		uint32_t _last_tick = 0;
};
