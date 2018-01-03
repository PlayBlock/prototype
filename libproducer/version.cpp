
#include <fc/exception/exception.hpp>
#include <fc/variant.hpp>

#include <sstream>

#include <libproducer/version.hpp>

namespace dev {
	namespace eth {
		namespace chain {

			/* Quick conversion utilities from http://joelverhagen.com/blog/2010/11/convert-an-int-to-a-string-and-vice-versa-in-c/ */
			inline int string_to_int(fc::string input)
			{
				std::stringstream s(input);
				int i;
				s >> i;
				return i;
			}

			inline fc::string int_to_string(int input)
			{
				std::stringstream s;
				s << input;
				return s.str();
			}

			version::version(uint8_t m, uint8_t h, uint16_t r)
			{
				v_num = (0 | m) << 8;
				v_num = (v_num | h) << 16;
				v_num = v_num | r;
			}

			version::operator fc::string()const
			{
				std::stringstream s;
				s << ((v_num >> 24) & 0x000000FF)
					<< '.'
					<< ((v_num >> 16) & 0x000000FF)
					<< '.'
					<< ((v_num & 0x0000FFFF));

				return s.str();
			}

}}}// eth::chain

