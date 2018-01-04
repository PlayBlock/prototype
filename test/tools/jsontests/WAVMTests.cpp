#include "DposTests.h"
#include <boost/range/algorithm/find.hpp>
using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

namespace dev {
	namespace test {

		Address newAddress(const Account& account)
		{
			return right160(sha3(rlpList(Address(account.address), u256(account.nonce))));
		}

		string WAVMAddress = "0x0000000000000000000000000000000000000020";
		string ZEROAddress = "0x0000000000000000000000000000000000000000";
		string code1 = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002a2010b03656e760a5741564d417373657274000303656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760b726561644d657373616765000103656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000b0f5f5a6d69524b35435432353653315f000c0f5f5a6d6c524b35435432353653315f000d0f5f5a6476524b35435432353653315f000e0f5f5a726d524b35435432353653315f000f0f5f5a6c74524b35435432353653315f00100f5f5a6774524b35435432353653315f00110f5f5a6571524b35435432353653315f0012125f5a3966756e6374696f6e313550617261310013125f5a3966756e6374696f6e32355061726132001404696e69740015056170706c7900160ab2040c0a0020012002200010080b0a0020012002200010080b0a0020012002200010060b0a0020012002200010010b0a0020012002200010050b0b002000200110044100470b0b002000200110034100470b0b002000200110024100470b0600200010090b0e0020002802002000280204100a0b0700418a3410090bb60303027f047e017f4100410028020441106b220736020442002104423b2103411021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d00200741086a4104100741034b41201000200728020810090b42002104423b210341c00021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d0020074108100741074b4120100020072802002007280204100a0b4100200741106a3602040b0b4e040041040b04504000000041100b0b746573745f66756e6331000041200b1e6d6573736167652073686f72746572207468616e206578706563746564000041c0000b0b746573745f66756e633200";

		string test_func1_string = "00c01eae1a4067a1";
		string test_func2_string = "00001fae1a4067a1";
		string para1_string = "15030000";
		string para2_string = "672b0000ce560000";

		BOOST_FIXTURE_TEST_SUITE(ContractTestsSuite, TestOutputHelperFixture)
			BOOST_AUTO_TEST_CASE(ctCreateContract)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];

			string gasLimit = "0xc350";
			string gasPrice = "0x04a817c800";
			string value = "0x0";
			string data = "";

			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, data, account);
			client.sendTransaction(gasLimit, gasPrice, ZEROAddress, value, data, account2);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;

			u256 balance2 = client.balance(Address(account2.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost2 = u256(1000000000000000000) - balance2;

			BOOST_REQUIRE(cost == cost2);
			string s_cost = cost.str();
			cout << "s_cost: " << s_cost << endl;

			gasLimit = "0x1f71b2";
			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			cout << "s_address: " << s_address << endl;
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);  //Create contract.
			client.produce_blocks();

			bytes contractCode = client.code(Address(s_address));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(code1) == 0);  //Check contract code.
			u256 balance3 = client.balance(Address(account.address));
			u256 cost_create = balance - balance3;
			string s_create = cost_create.str();
			cout << "s_create: " << s_create << endl;


			gasPrice = "0x09502f9000";  //Duplicate gasPrice.
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);  //Create contract.
			client.produce_blocks();
			u256 balance4 = client.balance(Address(account.address));
			u256 cost_create_2 = balance3 - balance4;
			string s_create_2 = cost_create_2.str();
			cout << "s_create_2: " << s_create_2 << endl;
			BOOST_REQUIRE(cost_create_2 == (cost_create * 2));  //Check cost.


			gasPrice = "0x04a817c800";  //Revert gasPrice.
			value = "0x04";  //Set endowment.
			Address m_newAddress_2 = newAddress(account);  //Contract Address.
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);  //Create contract.
			client.produce_blocks();
			u256 balance5 = client.balance(m_newAddress_2);
			cout << "contract endowment: " << balance5.str() << endl;
			BOOST_REQUIRE(balance5 == u256("4"));  //Check endowment.

		}

		BOOST_AUTO_TEST_CASE(ctCallContract)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";
			string data = "";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			cout << "s_address: " << s_address << endl;
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);  //Create contract.
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account.address));

			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string + para1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance4 = client.balance(Address(account.address));
			u256 cost_call = balance3 - balance4;
			string s_call = cost_call.str();
			cout << "s_call: " << s_call << endl;

			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string + para2_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance5 = client.balance(Address(account.address));
			u256 cost_call_2 = balance4 - balance5;
			string s_call_2 = cost_call_2.str();
			cout << "s_call_2: " << s_call_2 << endl;

		}

		BOOST_AUTO_TEST_SUITE_END()

	}
}