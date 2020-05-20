#pragma once
#include <vector>
#include <Utilities/Logger.h>

namespace Stardust::Network {
	typedef unsigned char byte;

	struct PacketOut {
		short ID;
		std::vector<byte> bytes;
	};
	struct PacketIn {
		short ID;
		short int pos;
		std::vector<byte> bytes;
	};

	inline std::vector<byte> encodeVarInt(int value) {
		std::vector<byte> vec;

		while (value > 127) {
			vec.push_back(((byte)(value & 127)) | 128);

			value >>= 7;
		}
		vec.push_back((byte)value & 127);
		return vec;
	}

	inline std::vector<byte> encodeVarIntLE(int value) {
		std::vector<byte> vec;

		while (value > 127) {
			vec.push_back(((byte)(value & 127)) | 128);

			value >>= 7;
		}
		vec.push_back((byte)value & 127);

		std::vector<byte> le;
		le.clear();
		
		for (int i = vec.size() - 1; i >= 0; i--) {
			le.push_back(vec[i]);
		}

		return le;
	}

	inline void encodeVarInt(int value, std::vector<byte>& p) {


		while (value > 127) {
			p.push_back(((byte)(value & 127)) | 128);

			value >>= 7;
		}
		p.push_back((byte)value & 127);
	}

	inline void encodeVarIntLE(int value, std::vector<byte>& p) {

		std::vector<byte> le;
		le.clear();

		while (value > 127) {
			le.push_back(((byte)(value & 127)) | 128);

			value >>= 7;
		}
		le.push_back((byte)value & 127);


		for (int i = le.size() - 1; i >= 0; i--) {
			p.push_back(le[i]);
		}
	}

	inline unsigned int decodeVarInt(std::vector<byte> input) {
		unsigned int result = 0;
  		unsigned int bits = 0;

		int pos = 0;
		
		while (input[pos] & 0x80) {
    		byte b = input[pos];
    		result += ((b & 0x7F) << bits);
    		pos++;
    		bits += 7;
  		}
		  
  		byte b = input[pos++];
  		result += ((b & 0x7F) << bits);

		return result;
	}

	inline unsigned int decodeVarInt(PacketIn& p) {
		unsigned int result = 0;
  		unsigned int bits = 0;
		
		while (p.bytes[p.pos] & 0x80) {
    		byte b = p.bytes[p.pos];
    		result += ((b & 0x7F) << bits);
    		p.pos++;
    		bits += 7;
  		}

  		byte b = p.bytes[p.pos++];
  		result += ((b & 0x7F) << bits);

		return result;
	}
}