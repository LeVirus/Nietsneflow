#include <fstream>
#include <sstream>
#include <iostream>

//ENCRYPT_KEY_CONF_FILE
//const uint32_t KEY = 42;
//STANDARD LEVEL
//const uint32_t KEY = 17;
//CUSTOM LEVEL
const uint32_t KEY = 52;

//===================================================================
std::string decrypt(const std::string &str, uint32_t key)
{
		std::string strR = str;
		for(uint32_t i = 0; i < strR.size(); ++i)
		{
				strR[i] -= key;
		}
		return strR;
}


//===================================================================
std::string encrypt(const std::string &str, uint32_t key)
{
		std::string strR = str;
		for(uint32_t i = 0; i < strR.size(); ++i)
		{
				strR[i] += key;
		}
		return strR;
}

int main(int argc, char *argv[])
{
		//if(argc != 2)
		//{
		//	std::cout << "Bad num Args\n";
		//	return -1;
		//}
		//std::string path = argv[1];
		std::ifstream inStream("./Ressources/CustomLevels/ff.clvl");
		//std::ifstream inStream("./Ressources/fontStandard.ini.base");
		//std::ifstream inStream("./Ressources/fontStandard.ini.base");
		//std::cout << "./Ressources/" + path + "/level.ini.dd \n";
		//std::ifstream inStream("./Ressources/" + path + "/level.ini.dec");
//		std::ifstream inStream("./Ressources/standardData.ini.base");
		if(inStream.fail())
		{
				std::cout << "Fail\n";
				inStream.close();
				return -1;
		}
		std::ostringstream ostringStream;
		ostringStream << inStream.rdbuf();
		inStream.close();
		std::string dataString = decrypt(ostringStream.str(), KEY);

		//std::ofstream outStream("./Ressources/fontData.ini");
		std::ofstream outStream("./Ressources/CustomLevels/FabbDec");
		//std::ofstream outStream("./Ressources/" + path + "/level.ini");
		//std::ofstream outStream("./Ressources/standardData.ini");
		outStream << dataString;
		outStream.close();
		std::cout << "OK\n";
		return 0;
}
