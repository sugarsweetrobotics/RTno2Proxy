#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "transport.h"
#include "profile.h"
#include "logger.h"
#include "result.h"
#include "state.h"
#include "ec_type.h"
#include "SerialDevice.h"

namespace ssr::rtno2
{

	static const size_t MAX_PACKET_SIZE = 255;
	template <typename T>
	inline T into(const uint8_t *buf, const uint8_t size)
	{
		return *(T *)buf;
	}

	template <typename T>
	inline std::vector<T> into_vec(const uint8_t *buf, const uint8_t size)
	{
		int length = size / sizeof(T);
		std::vector<T> val;
		for (int i = 0; i < length; i++)
		{
			val.push_back(into<T>(buf + i * sizeof(T), sizeof(T)));
		}
		return val;
	}

	union intermediate_float_uint8
	{
		uint8_t uint8_value[4];
		float float_value;
	};

	template <>
	inline float into<float>(const uint8_t *buf, const uint8_t size)
	{
		intermediate_float_uint8 i;
		memcpy(i.uint8_value, buf, size);
		return i.float_value;
	}

	union intermediate_double_uint8
	{
		uint8_t uint8_value[8];
		double double_value;
	};

	template <>
	inline double into<double>(const uint8_t *buf, const uint8_t size)
	{
		intermediate_double_uint8 i;
		memcpy(i.uint8_value, buf, size);
		return i.double_value;
	}

	class RTnoRTObjectWrapper;

	class protocol_t
	{
	private:
		transport_t transport_;
		logger_t logger_;

	public:
	public:
		protocol_t(ssr::SerialDevice *serial_device);
		virtual ~protocol_t(void);

	private:
		result_t<packet_t> wait_and_receive_command(const COMMAND command, const uint32_t wait_usec);

	public:
		result_t<profile_t> get_profile(const uint32_t wait_usec, const int retry_count = 5);
		result_t<STATE> get_state(const uint32_t wait_usec, const int retry_count = 5);
		result_t<EC_TYPE> get_ec_type(const uint32_t wait_usec, const int retry_count = 5);

		RESULT activate(const uint32_t wait_usec, const int retry_count = 5);
		RESULT deactivate(const uint32_t wait_usec, const int retry_count = 5);
		RESULT execute(const uint32_t wait_usec, const int retry_count = 5);

		RESULT send_inport_data(const std::string &portName, const uint8_t *data, const uint8_t length, const uint32_t wait_usec = 1000 * 1000, const int retry_count = 5);

		RESULT receive_outport_data(const std::string &portName, uint8_t *data, const uint8_t max_size, uint8_t *size_read, const uint32_t wait_usec = 1000 * 1000, const int retry_count = 5);

		template <typename T>
		result_t<T> receive_as(const std::string &portName)
		{
			static const uint8_t BUFSIZE = MAX_PACKET_SIZE;
			uint8_t size;
			uint8_t buffer[BUFSIZE];
			auto state = this->receive_outport_data(portName, buffer, BUFSIZE, &size);
			if (state == RESULT::OK)
			{
				return into<T>(buffer, size);
			}
			return state;
		}

		template <typename T>
		result_t<std::vector<T>> receive_seq_as(const std::string &portName)
		{
			const uint8_t BUFSIZE = MAX_PACKET_SIZE;
			uint8_t size;
			uint8_t buffer[BUFSIZE];
			auto state = this->receive_outport_data(portName, buffer, BUFSIZE, &size);
			if (state == RESULT::OK)
			{
				return into_vec<T>(buffer, size);
			}
			return state;
		}

	private:
		port_profile_t parse_port_profile(const packet_t &packet);
	};
}
