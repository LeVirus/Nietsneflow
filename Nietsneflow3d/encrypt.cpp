#include <fstream>
#include <sstream>
#include <iostream>

//ENCRYPT_KEY_CONF_FILE
const uint32_t KEY = 42;
//STANDARD LEVEL
//const uint32_t KEY = 17;

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
			//std::cout << "Bad num Args\n";
			//return -1;
		//}
		//std::string path = argv[1];
		std::ifstream inStream("./Ressources/fontData.ini.base");
		//std::ifstream inStream("./Ressources/pictureData.ini.base");
		//std::ifstream inStream("./Ressources/" + path + ".base");
		//std::ifstream inStream("./Ressources/standardData.ini.base");
		if(inStream.fail())
		{
				std::cout << "Fail\n";
				inStream.close();
				return -1;
		}
		std::ostringstream ostringStream;
		ostringStream << inStream.rdbuf();
		inStream.close();
		std::string dataString = encrypt(ostringStream.str(), KEY);

		std::ofstream outStream("./Ressources/fontData.ini");
		//std::ofstream outStream("./Ressources/pictureData.ini");
		//std::ofstream outStream("./Ressources/" + path);
		//std::ofstream outStream("./Ressources/standardData.ini");
		outStream << dataString;
		outStream.close();
		std::cout << "OK\n";
		return 0;
}
