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
		string code1 = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002a2010b03656e760a5741564d417373657274000303656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760b726561644d657373616765000103656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000b0f5f5a6d69524b35435432353653315f000c0f5f5a6d6c524b35435432353653315f000d0f5f5a6476524b35435432353653315f000e0f5f5a726d524b35435432353653315f000f0f5f5a6c74524b35435432353653315f00100f5f5a6774524b35435432353653315f00110f5f5a6571524b35435432353653315f0012125f5a3966756e6374696f6e313550617261310013125f5a3966756e6374696f6e32355061726132001404696e69740015056170706c7900160ab2040c0a0020012002200010080b0a0020012002200010080b0a0020012002200010060b0a0020012002200010010b0a0020012002200010050b0b002000200110044100470b0b002000200110034100470b0b002000200110024100470b0600200010090b0e0020002802002000280204100a0b0700418a3410090bb60303027f047e017f4100410028020441106b220736020442002104423b2103411021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d00200741086a4104100741034b41201000200728020810090b42002104423b210341c00021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d0020074108100741074b4120100020072802002007280204100a0b4100200741106a3602040b0b4e040041040b04504000000041100b0b746573745f66756e6331000041200b1e6d6573736167652073686f72746572207468616e206578706563746564000041c0000b0b746573745f66756e633200";
		string code2 = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002a2010b03656e760a5741564d417373657274000303656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760b726561644d657373616765000103656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000b0f5f5a6d69524b35435432353653315f000c0f5f5a6d6c524b35435432353653315f000d0f5f5a6476524b35435432353653315f000e0f5f5a726d524b35435432353653315f000f0f5f5a6c74524b35435432353653315f00100f5f5a6774524b35435432353653315f00110f5f5a6571524b35435432353653315f0012125f5a3966756e6374696f6e313550617261310013125f5a3966756e6374696f6e32355061726132001404696e69740015056170706c7900160a84060c0a0020012002200010080b0a0020012002200010080b0a0020012002200010060b0a0020012002200010010b0a0020012002200010050b0b002000200110044100470b0b002000200110034100470b0b002000200110024100470b0600200010090b0e0020002802002000280204100a0b0700418a3410090b880503027f047e017f4100410028020441206b220736020442002104423b2103411021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d00200741186a4104100741034b41201000200728021810090b42002104423b210341c00021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d00200741106a4108100741074b4120100020072802102007280214100a0b42002104423b210341d00021024200210503400240024002400240024020044209560d0020022c00002201419f7f6a41ff017141194b0d01200141a0016a21010c020b420021062004420b580d020c030b200141ea016a41002001414f6a41ff01714105491b21010b2001ad42388642388721060b2006421f83200342ffffffff0f838621060b200241016a2102200442017c2104200620058421052003427b7c2203427a520d000b024020052000520d00200741086a4108100741074b412010002007280208200728020c100a0b4100200741206a3602040b0b5f050041040b04604000000041100b0b746573745f66756e6331000041200b1e6d6573736167652073686f72746572207468616e206578706563746564000041c0000b0b746573745f66756e6332000041d0000b0b746573745f66756e633300";
		string code_initIO = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002a1010b03656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760873657473746f7265000303656e760c7365747532353676616c7565000303656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000b0f5f5a6d69524b35435432353653315f000c0f5f5a6d6c524b35435432353653315f000d0f5f5a6476524b35435432353653315f000e0f5f5a726d524b35435432353653315f000f0f5f5a6c74524b35435432353653315f00100f5f5a6774524b35435432353653315f00110f5f5a6571524b35435432353653315f0012125f5a3966756e6374696f6e313550617261310013125f5a3966756e6374696f6e32355061726132001404696e69740015056170706c7900160ab2010c0a0020012002200010080b0a0020012002200010080b0a0020012002200010050b0a0020012002200010000b0a0020012002200010040b0b002000200110034100470b0b002000200110024100470b0b002000200110014100470b0600200010090b0e0020002802002000280204100a0b3c01017f4100410028020441d0006b2200360204418a341009200041286a41101007200041201007200041286a200010064100200041d0006a3602040b02000b0b18030041040b04304000000041100b0236000041200b023700";
		string code_initIO_while = "0061736d01000000011d0660037f7f7f0060027f7f017f60017f0060027f7f0060000060017e0002a1010b03656e76086469765f75323536000003656e760765715f75323536000103656e760767745f75323536000103656e76076c745f75323536000103656e76086d6f645f75323536000003656e76086d756c5f75323536000003656e760873657473746f7265000303656e760c7365747532353676616c7565000303656e76087375625f75323536000003656e76057465737431000203656e760574657374320003030d0c000000000001010102020405040401700000050301000107d3010d066d656d6f727902000f5f5a706c524b35435432353653315f000b0f5f5a6d69524b35435432353653315f000c0f5f5a6d6c524b35435432353653315f000d0f5f5a6476524b35435432353653315f000e0f5f5a726d524b35435432353653315f000f0f5f5a6c74524b35435432353653315f00100f5f5a6774524b35435432353653315f00110f5f5a6571524b35435432353653315f0012125f5a3966756e6374696f6e313550617261310013125f5a3966756e6374696f6e32355061726132001404696e69740015056170706c7900160aac010c0a0020012002200010080b0a0020012002200010080b0a0020012002200010050b0a0020012002200010000b0a0020012002200010040b0b002000200110034100470b0b002000200110024100470b0b002000200110014100470b0600200010090b0e0020002802002000280204100a0b3601017f4100410028020441d0006b2200360204418a341009200041286a411010072000412010070340200041286a200010060c000b0b02000b0b18030041040b04304000000041100b0236000041200b023700";

		string test_func1_string = "00c01eae1a4067a1";
		string test_func2_string = "00001fae1a4067a1";
		string para1_string = "15030000";
		string para2_string = "672b0000ce560000";

		BOOST_FIXTURE_TEST_SUITE(ContractTestsSuite, TestOutputHelperFixture)
		BOOST_AUTO_TEST_CASE(ctCreateContract)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0xc350";
			string gasPrice = "0x04a817c800";
			string value = "0x0";
			string data = "";

			//创建空合约
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, data, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			string s_cost = cost.str();
			cout << "s_cost: " << s_cost << endl;

			//普通创建合约
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

			//gasPrice翻倍，创建相同的合约
			gasPrice = "0x09502f9000";  //Duplicate gasPrice.
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);  //Create contract.
			client.produce_blocks();
			u256 balance4 = client.balance(Address(account.address));
			u256 cost_create_2 = balance3 - balance4;
			string s_create_2 = cost_create_2.str();
			cout << "s_create_2: " << s_create_2 << endl;
			BOOST_REQUIRE(cost_create_2 == (cost_create * 2));  //Check cost.

			//恢复gasPrice，创建合约时，给合约转账
			gasPrice = "0x04a817c800";  //Revert gasPrice.
			value = "0x04";  //Set endowment.
			Address m_newAddress_2 = newAddress(account);  //Contract Address.
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);  //Create contract.
			client.produce_blocks();
			u256 balance5 = client.balance(m_newAddress_2);
			cout << "contract endowment: " << balance5.str() << endl;
			BOOST_REQUIRE(balance5 == u256(4));  //Check endowment.

			//创建合约时“to”字段有值，表示普通账户，该地址没有code，“value”字段有值。
			//创建失败，解析为合约调用，“value”的值会转给“to”地址。
			Address m_newAddress_3 = newAddress(account);  //Contract Address.
			client.sendTransaction(gasLimit, gasPrice, account2.address, value, "0x" + code1, account);  //Create contract. (Error)
			client.produce_blocks();

			u256 balance6 = client.balance(m_newAddress_3);
			cout << "contract endowment: " << balance6.str() << endl;
			BOOST_REQUIRE(balance6 == u256());  //Check endowment.

			u256 balance7 = client.balance(Address(account2.address));
			cout << "contract endowment: " << balance7.str() << endl;
			BOOST_REQUIRE(balance7 - u256(1000000000000000000) == u256(4));  //Check endowment.

			bytes contractCode_2 = client.code(m_newAddress_3);  //Get contract address.
			BOOST_REQUIRE(contractCode_2.size() == 0);
			bytes contractCode_3 = client.code(Address(account2.address));
			BOOST_REQUIRE(contractCode_3.size() == 0);

			//创建合约时“to”字段有值，表示普通账户，该地址没有code，“value”字段为零。
			//创建失败，解析为合约调用。
			value = "0x00";
			Address m_newAddress_4 = newAddress(account);  //Contract Address.
			client.sendTransaction(gasLimit, gasPrice, account3.address, value, "0x" + code1, account);  //Create contract. (Error)
			client.produce_blocks();

			u256 balance8 = client.balance(m_newAddress_4);
			cout << "contract endowment: " << balance8.str() << endl;
			BOOST_REQUIRE(balance8 == u256());  //Check endowment.
			u256 balance9 = client.balance(Address(account3.address));
			cout << "contract endowment: " << balance9.str() << endl;
			BOOST_REQUIRE(balance9 == u256(1000000000000000000));  //Check endowment.
			bytes contractCode_4 = client.code(m_newAddress_4);  //Get contract address.
			BOOST_REQUIRE(contractCode_4.size() == 0);
			bytes contractCode_5 = client.code(Address(account3.address));
			BOOST_REQUIRE(contractCode_5.size() == 0);

		}

		BOOST_AUTO_TEST_CASE(ctCodeSize)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建短合约
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			string s_cost = cost.str();
			cout << "s_cost: " << s_cost << endl;

			//创建长合约
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code2, account2);
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account2.address));
			BOOST_REQUIRE(balance2 < u256(1000000000000000000));
			u256 cost2 = u256(1000000000000000000) - balance2;
			string s_cost2 = cost2.str();
			cout << "s_cost2: " << s_cost2 << endl;

			BOOST_REQUIRE(cost < cost2);

			//调整交易的gasLimit，模拟发布合约代码过长，超过块最大gasLimit限制
			gasLimit = "0x1f71b20000";
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code2, account2);
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account2.address));
			u256 cost3 = balance2 - balance3;
			string s_cost3 = cost3.str();
			cout << "s_cost3: " << s_cost3 << endl;

			BOOST_REQUIRE(cost3  == u256());
		}

		BOOST_AUTO_TEST_CASE(ctLakeBalance)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//验证io可用
			Address m_newAddress = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code_initIO, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			cout << "cost.str(): " << cost.str() << endl;

			u256 res1 = client.storage(m_newAddress, u256(6));  //Get contract storage.
			BOOST_REQUIRE(res1 == u256(7));
			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(code_initIO) == 0);  //Check contract code.

			//运行init时，gas不足。合约发布失败，扣除所有gas。
			Address m_newAddress2 = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code_initIO_while, account2);
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account2.address));
			u256 cost2 = u256(1000000000000000000) - balance2;
			BOOST_REQUIRE(cost2 == u256(2060722) * u256(20000000000));
			BOOST_REQUIRE(cost2 > cost);
			cout << "cost2.str(): " << cost2.str() << endl;

			u256 res2 = client.storage(m_newAddress2, u256(6));  //Get contract storage.
			BOOST_REQUIRE(res2 == u256());
			bytes contractCode_2 = client.code(Address(m_newAddress2));  //Get contract code.
			BOOST_REQUIRE(contractCode_2.size() == 0);

			//增大gasPrice，使地址余额小于gas*gasPrice。合约发布失败，交易没有被执行，不扣除gas。
			gasPrice = "0x04a817c80000000000";
			Address m_newAddress3 = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code_initIO, account3);
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account3.address));
			BOOST_REQUIRE(balance3 == u256(1000000000000000000));

			u256 res3 = client.storage(m_newAddress3, u256(6));  //Get contract storage.
			BOOST_REQUIRE(res3 == u256());
			bytes contractCode_3 = client.code(Address(m_newAddress3));  //Get contract code.
			BOOST_REQUIRE(contractCode_3.size() == 0);

		}

		BOOST_AUTO_TEST_CASE(ctCompile)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建非法合约
			Address m_newAddress = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1.substr(0, 100), account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			u256 cost = u256(1000000000000000000) - balance;
			cout << "cost.str(): " << cost.str() << endl;
			BOOST_REQUIRE(cost == u256(2060722) * u256(20000000000));
			bytes contractCode_2 = client.code(Address(m_newAddress));  //Get contract code.
			BOOST_REQUIRE(contractCode_2.size() == 0);
		}

		BOOST_AUTO_TEST_CASE(ctLakeField)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 3);
			Account& account = client.get_accounts()[0];
			Account& account2 = client.get_accounts()[1];
			Account& account3 = client.get_accounts()[2];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";

			//创建短合约，缺少“to”字段
			Address m_newAddress = newAddress(account);
			client.sendTransaction(gasLimit, gasPrice, "", value, "0x" + code1, account);
			client.produce_blocks();

			u256 balance = client.balance(Address(account.address));
			BOOST_REQUIRE(balance < u256(1000000000000000000));
			u256 cost = u256(1000000000000000000) - balance;
			cout << "cost.str(): " << cost.str() << endl;

			bytes contractCode = client.code(Address(m_newAddress));  //Get contract address.
			string s_contractCode = toHex(contractCode);
			BOOST_REQUIRE(s_contractCode.compare(code1) == 0);

			//创建短合约，缺少“gasLimit”字段
			Address m_newAddress_2 = newAddress(account);
			client.sendTransaction("", gasPrice, "", value, "0x" + code1, account);
			client.produce_blocks();

			u256 balance2 = client.balance(Address(account.address));
			BOOST_REQUIRE(balance == balance2);

			bytes contractCode_2 = client.code(Address(m_newAddress_2));  //Get contract address.
			BOOST_REQUIRE(contractCode_2.size() == 0);

			//创建短合约，缺少“gasPrice”字段
			Address m_newAddress_3 = newAddress(account2);
			client.sendTransaction(gasLimit, "", "", value, "0x" + code1, account2);
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account2.address));
			u256 cost_2 = u256(1000000000000000000) - balance3;
			string ss = cost_2.str();
		}

		BOOST_AUTO_TEST_CASE(ctInit)
		{

		}

		BOOST_AUTO_TEST_CASE(ctCallContract)
		{
			DposTestClient client;

			BOOST_REQUIRE(client.get_accounts().size() >= 2);
			Account& account = client.get_accounts()[0];

			string gasLimit = "0x1f71b2";
			string gasPrice = "0x04a817c800";
			string value = "0x0";


			Address m_newAddress = newAddress(account);  //Contract Address.
			string s_address = m_newAddress.hex();
			cout << "s_address: " << s_address << endl;
			client.sendTransaction(gasLimit, gasPrice, WAVMAddress, value, "0x" + code1, account);  //Create contract.
			client.produce_blocks();

			u256 balance3 = client.balance(Address(account.address));

			//调用合约方法1
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func1_string + para1_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance4 = client.balance(Address(account.address));
			u256 cost_call = balance3 - balance4;
			BOOST_REQUIRE(cost_call < u256(2060722) * u256(20000000000));
			string s_call = cost_call.str();
			cout << "s_call: " << s_call << endl;

			//调用合约方法2
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string + para2_string, account);  //Call contract code.
			client.produce_blocks();

			u256 balance5 = client.balance(Address(account.address));
			u256 cost_call_2 = balance4 - balance5;
			BOOST_REQUIRE(cost_call_2 < u256(2060722) * u256(20000000000));
			string s_call_2 = cost_call_2.str();
			cout << "s_call_2: " << s_call_2 << endl;

			//调用函数散列值太短
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string.substr(0, 14), account);  //Call contract code. (Error)
			client.produce_blocks();

			u256 balance6 = client.balance(Address(account.address));
			u256 cost_call_3 = balance5 - balance6;
			BOOST_REQUIRE(cost_call_3 == u256(2060722) * u256(20000000000));

			//调用函数参数不对
			client.sendTransaction(gasLimit, gasPrice, s_address, value, test_func2_string + para1_string, account);  //Call contract code. (Error)
			client.produce_blocks();

			u256 balance7 = client.balance(Address(account.address));
			u256 cost_call_4 = balance6 - balance7;
			BOOST_REQUIRE(cost_call_4 == u256(2060722) * u256(20000000000));

		}

		BOOST_AUTO_TEST_SUITE_END()

	}
}