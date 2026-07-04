#pragma once
#include <inttypes.h>
#include <DrakePinD.hpp>

template <uint8_t _count = 0, uint16_t _tick_time = 25> 
class AnalogMux
{
	//static constexpr uint8_t _count = 4;
	//static constexpr uint16_t _tick_time = 25;
	
	using func_adc_req_t = uint16_t (*)(uint8_t address);
	using func_result_t = void (*)(uint8_t address, uint16_t value);
	
	public:
		
		template <typename... DPinTs> 
		AnalogMux(func_adc_req_t adc_req, func_result_t result, DPinTs... dpins) : 
			_CallbackGetADC(adc_req), 
			_CallbackResult(result), 
			_dpins{ DrakePinD(dpins, DrakePin::Output, DrakePin::Low)... }, 
			_adc_value{}
		{
			static_assert(sizeof...(dpins) == _count, "Incorrect number of digital pins provided");
		}
		
		void Init()
		{
			for(auto &dpin : _dpins)
			{
				dpin.Init();
			}
			
			return;
		}
		
		// Получить последнее значение ADC
		// uint8_t port - Номер порта, начиная с 1
		uint16_t Get(uint8_t port)
		{
			if(--port >= (1 << _count)) return 0;
			
			return _adc_value[port];
		}
		
		// Прочитать и получить значение ADC
		// uint8_t port - Номер порта, начиная с 1
		uint16_t Read(uint8_t port)
		{
			if(--port >= (1 << _count)) return 0;
			
			_Read(port);
			
			return _adc_value[port];
		}
		
		void Processing(uint32_t time)
		{
			if(time - _last_tick < _tick_time) return;
			_last_tick = time;
			
			for(uint8_t addr = 0; addr < (1 << _count); ++addr)
			{
				_Read(addr);
				_CallbackResult((addr + 1), _adc_value[addr]);
			}
			
			return;
		}
		
	private:
		void _SetAddr(uint8_t address)
		{
			for(uint8_t i = 0; i < _count; ++i)
			{
				if(address & (1 << i)){
					_dpins[i].On();
				} else {
					_dpins[i].Off();
				}
			}
			
			return;
		}
		
		void _Read(uint8_t address)
		{
			_SetAddr(address);
			asm("nop; nop; nop; \n");
			_adc_value[address] = _CallbackGetADC((address + 1));
			
			return;
		}
		
		func_adc_req_t _CallbackGetADC;
		func_result_t _CallbackResult;
		DrakePinD _dpins[_count];
		uint16_t _adc_value[(1 << _count)];
		
		uint32_t _last_tick = 0;
};
