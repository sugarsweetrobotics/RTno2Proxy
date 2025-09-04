
#include "rtno2/Packet.h"
#include "rtno2/protocol.h"

#include <chrono>
#include <thread>
#include <iostream>

#include "rtno2/logger.h"


using namespace ssr::rtno2;


protocol_t::protocol_t(SerialDevice* serial_device) :
	transport_(serial_device), logger_(get_logger("RTnoProtocol")) {

	set_log_level(&logger_, LOGLEVEL::TRACE);
	RTNO_TRACE(logger_, "RTnoProtocol() called");
}

protocol_t::~protocol_t(void)
{
}

result_t<packet_t> protocol_t::wait_and_receive_command(const COMMAND command, const uint32_t wait_usec) {
	RTNO_TRACE(logger_, "protocol_t::wait_and_receive_command({}, {}) called", command_to_string(command), wait_usec);
	RESULT result;
	if ((result = transport_.is_new(wait_usec)) == RESULT::OK) {
		auto receive_result = transport_.receive(wait_usec);
		if (receive_result.result != RESULT::OK) {
			RTNO_ERROR(logger_, "wait_an_receive_command failed. {}", result_to_string(receive_result.result));
			return receive_result.result;
		}

		packet_t pac = receive_result.value.value();
		RTNO_DEBUG(logger_, "wait_and_receive_command {}", pac.to_string());
		if (pac.get_command() == command) {
			RTNO_TRACE(logger_, "wait_and_receive_command() exit with success");
			return pac;
		}
		
		if (pac.get_command() == COMMAND::PACKET_ERROR) {
			RTNO_ERROR(logger_, "protocol::wait_and_receive_command() detect PACKET_ERROR. Replied messasge includes RESULT({})", result_to_string(pac.get_result()));
			return pac.get_result();
		}
		else {
			RTNO_ERROR(logger_, "RTnoProtocol::waitCommand() still wait for unknown command({}/{})", ((char)pac.get_command()), ((int)pac.get_command()));
			return RESULT::ERR;
		}
	}
	RTNO_TRACE(logger_, "wait_and_receive_command() exit");
	return result;
}


result_t<STATE> protocol_t::get_state(uint32_t wait_usec, const int retry_count) {
	RTNO_TRACE(logger_, "RTnoProtocol::getRTnoState() called.");
	static const packet_t cmd_packet(COMMAND::GET_STATE, RESULT::OK);
	for (int i = 0; i < retry_count; i++) {

		transport_.send(cmd_packet);

		auto result = wait_and_receive_command(COMMAND::GET_STATE, wait_usec);
		if (result.result == RESULT::OK) {

			RTNO_TRACE(logger_, "RTnoProtocol::getRTnoState() exit");
			return (STATE)result.value.value().getData()[0];
		}
		else if (result.result == RESULT::PACKET_START_TIMEOUT || result.result == RESULT::PACKET_HEADER_TIMEOUT || result.result == RESULT::PACKET_BODY_TIMEOUT || result.result == RESULT::PACKET_CHECKSUM_TIMEOUT) {
			RTNO_DEBUG(logger_, "In getRTnoState(), timeout. retry");
		}
	}

	RTNO_TRACE(logger_, "RTnoProtocol::getRTnoState() exit");
	return RESULT::ERR;
}

result_t<EC_TYPE> protocol_t::get_ec_type(uint32_t wait_usec, const int retry_count) {
	RTNO_TRACE(logger_, "RTnoProtocol::getRTnoExecutionContextType() called.");
	static const packet_t cmd_packet(COMMAND::GET_CONTEXT_TYPE, RESULT::OK);
	for (int i = 0; i < retry_count; i++) {

		transport_.send(cmd_packet);
		auto result = wait_and_receive_command(COMMAND::GET_CONTEXT_TYPE, wait_usec);
		if (result.result == RESULT::OK) {
			RTNO_TRACE(logger_, "RTnoProtocol::getRTnoExecutionContextType() exit");
			return (EC_TYPE)result.value.value().getData()[0];
		}
		else if (result.result == RESULT::PACKET_START_TIMEOUT || result.result == RESULT::PACKET_HEADER_TIMEOUT || result.result == RESULT::PACKET_BODY_TIMEOUT || result.result == RESULT::PACKET_CHECKSUM_TIMEOUT) {
			RTNO_DEBUG(logger_, "In getRTnoState(), timeout. retry");
		}
	}
	RTNO_TRACE(logger_, "RTnoProtocol::getRTnoExecutionContextType() exit");
	return RESULT::ERR;
}


result_t<profile_t> protocol_t::get_profile(const uint32_t wait_usec, const int retry_count) {
	RTNO_TRACE(logger_, "RTnoProtocol::getRTnoProfile() called.");
	static const packet_t cmd_packet(COMMAND::GET_PROFILE, (RESULT::OK));
	for (int i = 0; i < retry_count; i++) {

		transport_.send(cmd_packet);
		profile_t profile;

		int timeout_count = wait_usec / 1000;
		while (1) {
			auto result = transport_.is_new(wait_usec);
			if (result != RESULT::OK) {

				RTNO_WARN(logger_, "RTnoProtocol::getRTnoProfile() isNew timeout.");
				break;
			}

			auto receive_result = transport_.receive(wait_usec);
			if (receive_result.result != RESULT::OK) {
				if (receive_result.result == RESULT::PACKET_START_TIMEOUT || receive_result.result == RESULT::PACKET_HEADER_TIMEOUT || receive_result.result == RESULT::PACKET_BODY_TIMEOUT || receive_result.result == RESULT::PACKET_CHECKSUM_TIMEOUT) {
					RTNO_DEBUG(logger_, "In getRTnoState(), timeout. retry");
					break;
				}
				else {
					RTNO_ERROR(logger_, "In get_profile, receive error: {}", result_to_string(receive_result.result));
					return receive_result.result;
				}
			}

			packet_t pac = receive_result.value.value();
			switch ((COMMAND)pac.get_command()) {
			case COMMAND::GET_PROFILE: // Return Code.
				RTNO_DEBUG(logger_, "COMMAND::GET_PROFILE received ({})", profile.to_string());
				RTNO_TRACE(logger_, "RTnoProtocol::getRTnoProfile() exit");
				return profile; // onGetProfile(pac);

			case COMMAND::INPORT_PROFILE:
			{
				auto inport_prof = parse_port_profile(pac);
				RTNO_DEBUG(logger_, "COMMAND::INPORT_PROFILE received ({})", inport_prof.to_string());
				profile.append_in_port(inport_prof);
			}
			break;

			case COMMAND::OUTPORT_PROFILE:
			{
				auto outport_prof = parse_port_profile(pac);
				RTNO_DEBUG(logger_, "COMMAND::OUTPORT_PROFILE received ({})", outport_prof.to_string());
				profile.append_out_port(outport_prof);
			}
			break;

			case COMMAND::PACKET_ERROR:
			{
				RTNO_ERROR(logger_, "COMMAND::PACKET_ERROR received. (packet={})", pac.to_string());
				return RESULT::ERR;
			}
			case COMMAND::PACKET_ERROR_CHECKSUM:
				RTNO_ERROR(logger_, "COMMAND::PACKET_ERROR_CHECKSUM received.");
				return RESULT::CHECKSUM_ERROR;
			case COMMAND::PACKET_ERROR_TIMEOUT:

				RTNO_ERROR(logger_, "COMMAND::PACKET_ERROR_TIMEOUT received.");
				return RESULT::TIMEOUT;
			default:
				RTNO_ERROR(logger_, "Unknown Command ({})", command_to_string(pac.get_command()));
			}
		}
	}
	return RESULT::TIMEOUT;
}

// RTnoProfile RTnoProtocol::onGetProfile(const packet_t& packet)
// {
//   if (packet.getResult() != RESULT::OK) {
//     std::cout << "--RTnoProtocol::getRTnoProfile() Failed." << std::endl;
//     throw GetProfileException();
//   }
// }

port_profile_t protocol_t::parse_port_profile(const packet_t& packet) {
	char strbuf[64];
	memcpy(strbuf, packet.getData() + 1, packet.getDataLength() - 1);
	strbuf[packet.getDataLength() - 1] = 0;
	return port_profile_t((TYPECODE)packet.getData()[0], strbuf);
}


static bool is_ok(const ssr::rtno2::packet_t& packet) {
	return packet.get_result() == RESULT::OK;
}

RESULT protocol_t::activate(const uint32_t wait_usec, const int retry_count) {
	RTNO_TRACE(logger_, " - RTnoProtocol::activateRTno() called.");
	static const packet_t cmd_packet(COMMAND::ACTIVATE, RESULT::OK);
	for (int i = 0; i < retry_count; i++) {

		transport_.send(cmd_packet);
		auto result = wait_and_receive_command(COMMAND::ACTIVATE, wait_usec);
		if (result.result == RESULT::OK) {
			RTNO_DEBUG(logger_, " - RTnoProtocol::activateRTno() exit with success.");
			return result.result;
		}
		//      RTNO_DEBUG(m_Logger, " - RTnoProtocol::activateRTno() exit with failure.");

		else if (result.result == RESULT::PACKET_START_TIMEOUT || result.result == RESULT::PACKET_HEADER_TIMEOUT || result.result == RESULT::PACKET_BODY_TIMEOUT || result.result == RESULT::PACKET_CHECKSUM_TIMEOUT) {
			RTNO_DEBUG(logger_, "In getRTnoState(), timeout. retry");
		}
	}
	return RESULT::ERR;
}

RESULT protocol_t::deactivate(const uint32_t wait_usec, const int retry_count) {
	static const packet_t cmd_packet(COMMAND::DEACTIVATE, RESULT::OK);
	for (int i = 0; i < retry_count; i++) {

		transport_.send(cmd_packet);
		auto result = wait_and_receive_command(COMMAND::DEACTIVATE, wait_usec);
		if (result.result == RESULT::OK) {
			RTNO_DEBUG(logger_, " - RTnoProtocol::deactivateRTno() exit with success.");
			return result.result;
		}
		else if (result.result == RESULT::PACKET_START_TIMEOUT || result.result == RESULT::PACKET_HEADER_TIMEOUT || result.result == RESULT::PACKET_BODY_TIMEOUT || result.result == RESULT::PACKET_CHECKSUM_TIMEOUT) {
			RTNO_DEBUG(logger_, "In getRTnoState(), timeout. retry");
		}
	}
	return RESULT::ERR;
}

RESULT protocol_t::execute(const uint32_t wait_usec, const int retry_count) {
	RTNO_TRACE(logger_, "executeRTno() called");
	static const packet_t cmd_packet(COMMAND::EXECUTE, RESULT::OK);
	for (int i = 0; i < retry_count; i++) {

		transport_.send(cmd_packet);
		auto result = wait_and_receive_command(COMMAND::EXECUTE, wait_usec);
		if (result.result == RESULT::OK) {
			RTNO_DEBUG(logger_, "executeRTno() exit with {}", result_to_string(result.value.value().get_result()));
			return result.result;
		}
		else if (result.result == RESULT::PACKET_START_TIMEOUT || result.result == RESULT::PACKET_HEADER_TIMEOUT || result.result == RESULT::PACKET_BODY_TIMEOUT || result.result == RESULT::PACKET_CHECKSUM_TIMEOUT) {
			RTNO_DEBUG(logger_, "In getRTnoState(), timeout. retry");
		}
	}
	return RESULT::ERR;
}


RESULT protocol_t::send_inport_data(const std::string& portName, const uint8_t* data, const uint8_t length, const uint32_t wait_usec, const int retry_count) {

	RTNO_TRACE(logger_, "send_inport_data(port={}, retry={}) called", portName, retry_count);
	auto namelen = portName.length();
	uint8_t buffer[64];
	buffer[0] = static_cast<uint8_t>(namelen);
	buffer[1] = length;
	memcpy(buffer + 2, portName.c_str(), namelen);
	memcpy(buffer + 2 + namelen, data, length);
	packet_t packet(COMMAND::SEND_DATA, (RESULT)RESULT::OK, buffer, static_cast<uint8_t>(2 + namelen + length));
	transport_.send(packet);
	auto result = wait_and_receive_command(COMMAND::SEND_DATA, wait_usec);
	if (result.result == RESULT::OK) {
		RTNO_TRACE(logger_, "send_inport_data() exit with success ({})", result.value.value().to_string());
		return result.value.value().get_result();
	}
	RTNO_TRACE(logger_, "send_inport_data() exit with error {}", result_to_string(result.result));
	return RESULT::ERR;
}

RESULT protocol_t::receive_outport_data(const std::string& portName, uint8_t* data, const uint8_t max_size, uint8_t* size_read, const uint32_t wait_usec, const int retry_count) {

	RTNO_TRACE(logger_, "receiveData(portName={}, retry={}) called", portName, retry_count);
	auto namelen = portName.length();//strlen(portName);
	uint8_t buffer[64];
	buffer[0] = static_cast<uint8_t>(namelen);
	memcpy(buffer + 2, portName.c_str(), namelen);
	packet_t packet(COMMAND::RECEIVE_DATA, (RESULT)RESULT::OK, buffer, static_cast<uint8_t>(2 + namelen));
	transport_.send(packet);
	auto result = wait_and_receive_command(COMMAND::RECEIVE_DATA, wait_usec);
	if (result.result == RESULT::OK) {
		auto packet = result.value.value();
		int8_t name_len = packet.getData()[0];
		int8_t data_len = packet.getData()[1];
		*size_read = data_len;
		if (*size_read > max_size) {
			RTNO_ERROR(logger_, "receiveData() maximum buffer size exceeded (max={}, received={})", max_size, *size_read);
			RTNO_ERROR(logger_, "receiveData() exit with error");
			return packet.get_result();
		}
		memcpy(data, packet.getData() + 2 + name_len, *size_read);
		RTNO_TRACE(logger_, "receiveData() exit with success ({})", result.value.value().to_string());
		return packet.get_result();

	}

	RTNO_ERROR(logger_, "receiveData() exit with error");
	return RESULT::ERR;
}


// void RTnoProtocol::onAddInPort(const packet_t& packet) {
//   m_Profile.appendInPort(parsePortProfile(packet));
// }

// void RTnoProtocol::onAddOutPort(const packet_t& packet) {
//   m_Profile.appendOutPort(parsePortProfile(packet));
// }


#if 0 
uint8_t RTnoProtocol::getRTnoStatus() {
	static const packet_t cmd_packet(GET_STATUS);
	m_pTransport->send(cmd_packet);
	packet_t pac = waitCommand(GET_STATUS, 20 * 1000);
	return pac.getData()[0];
}

uint8_t RTnoProtocol::getRTnoExecutionContextType() {
	static const packet_t cmd_packet(GET_CONTEXT);
	m_pTransport->send(cmd_packet);
	packet_t pac = waitCommand(GET_CONTEXT, 20 * 1000);
	return pac.getData()[0];
}

uint8_t RTnoProtocol::activate() {
	static const packet_t cmd_packet(RTNO_ACTIVATE);
	m_pTransport->send(cmd_packet);
	packet_t pac = waitCommand(RTNO_ACTIVATE, 20 * 1000);
	return pac.getData()[0];
}

uint8_t RTnoProtocol::reset() {
	static const packet_t cmd_packet(RTNO_RESET);
	m_pTransport->send(cmd_packet);
	packet_t pac = waitCommand(RTNO_RESET, 20 * 1000);
	return pac.getData()[0];
}

uint8_t RTnoProtocol::deactivate() {
	static const packet_t cmd_packet(RTNO_DEACTIVATE);
	m_pTransport->send(cmd_packet);
	packet_t pac = waitCommand(RTNO_DEACTIVATE, 20 * 1000);
	return pac.getData()[0];
}

uint8_t RTnoProtocol::sendData(const std::string& portName, const uint8_t* data, const uint8_t length) {
	int namelen = portName.length();//strlen(portName);
	uint8_t buffer[64];
	buffer[0] = namelen;
	buffer[1] = length;
	memcpy(buffer + 2, portName.c_str(), namelen);
	memcpy(buffer + 2 + namelen, data, length);
	packet_t packet(SEND_DATA, buffer, 2 + namelen + length);
	m_pTransport->send(packet);
	return 0;
}


int32_t RTnoProtocol::sendExecuteTrigger(void) {
	static const packet_t cmd_packet(RTNO_EXECUTE);
	return m_pTransport->send(cmd_packet);
}

void RTnoProtocol::receiveData(const uint8_t* data)
{
	char name_buffer[16];
	memcpy(name_buffer, data + 2, data[0]); name_buffer[data[0]] = 0;
	OutPortWrapperBase* outPort = m_pRTObjectWrapper->GetOutPort(name_buffer);
	if (outPort == NULL) {
		throw UnknownOutPortRequestedException();
	}

	outPort->Write((void*)(data + 2 + data[0]), data[1] / outPort->getTypeSizeInArduino());
}

void RTnoProtocol::handleReceivedPacket(const uint32_t wait_usec) {

	if (m_ProxySynchronousExecution) {
		sendExecuteTrigger();
	}

	bool endFlag = false;
	while (!endFlag) {
		if (m_pTransport->isNew()) {
			try {
				packet_t pac = m_pTransport->receive(wait_usec);
				switch (pac.getInterface()) {
				case RECEIVE_DATA:
					//ReceiveData(packet_buffer);
					receiveData(pac.getData());
					break;
				case RTNO_EXECUTE:
					if (pac.getData()[0] != RTNO_OK) {
						throw ExecuteFailedException();
					}
					endFlag = true;
					break;
				case SEND_DATA:
					//this->m_SendBusy = FALSE;
					break;
				default:
					std::cout << "Unknown Packet: " << pac.getInterface() << std::endl;
					endFlag = true;
					break;
				}
			}
			catch (TimeOutException& ex) {
				if (!m_ProxySynchronousExecution) {
					throw ex;
				}
				else {
					break;
				}
			}
		}
	}
}

uint8_t RTnoProtocol::initialize()
{
	uint8_t status = getRTnoStatus();
	std::cout << MSGHDR << " - RTno Status=" << (int)status << std::endl;
	int ret;
	switch (status) {
	case STATE_ACTIVE:
		std::cout << MSGHDR << " -- Now Arduino is in ACTIVE STATE. Deactivating..." << std::endl;
		if ((ret = deactivate()) != 0) {
			std::cout << MSGHDR << " -- Failed." << std::endl;
			return false;
		}
		std::cout << MSGHDR << " -- OK." << std::endl;
		break;
	case STATE_INACTIVE:
		std::cout << MSGHDR << " -- Now Arduino is in INACTIVE STATE" << std::endl;
		break;
	case STATE_ERROR:
		std::cout << MSGHDR << " -- Now Arduino is in ERROR STATE. Resetting..." << std::endl;
		if ((ret = reset()) != 0) {
			std::cout << MSGHDR << " -- Failed." << std::endl;
			return false;
		}
		std::cout << MSGHDR << " -- OK." << std::endl;
		break;
	}

	uint8_t contextType = getRTnoExecutionContextType();
	std::cout << MSGHDR << "Execution Context Type == " << (int)contextType << std::endl;
	switch (contextType) {
	case ProxySynchronousExecutionContext:
		std::cout << MSGHDR << " - ProxySynchronousExecutionContext is detected." << std::endl;
		std::cout << MSGHDR << " - This Arduino's on_execute is synchronously executed with this PC.\n" << std::endl;
		std::cout << MSGHDR << " - You can change excution ratio by configuring RTnoProxy's one." << std::endl;
		m_ProxySynchronousExecution = true;
		break;
	default:
		std::cout << " - ProxySynchronousExecutionContext is NOT detected." << std::endl;
		std::cout << " - You can change exeecution ratio by implementing Arduino Code\n" << std::endl;
		m_ProxySynchronousExecution = false;
	}


	return 0;
}

#endif