//
//  Faust_Exporter.h 

 
#include <string>
#include <vector>
#include <map>

#include <sstream>
#include <iostream>
#include <fstream> 

#include <jack/net.h>
#include <curl/curl.h>

using namespace std;

/* Access to FaustWeb service -Sends a request to faustweb compilation service to know platforms and architecture supported by this
 *
 * @param url - FaustWeb server URL 
 * @param platforms - If the research is sucessfull, the vector is filled with available platforms
 * @param targets - Associates a platform with a vector of available architectures
 * @param error - In case the research fails, the error is filled
 *
 * @return true if no error was encountered
 */
bool get_available_targets(const std::string& url, std::vector<std::string>& platforms, std::map<std::string, std::vector<std::string> >& targets, string& error);


/* Access to FaustWeb service - Upload your faust application given a sourceFile, an operating system and an architecture
 *
 * @param url - FaustWeb server URL
 * @param file - Faust source file
 * @param os - Wanted Operating System
 * @param architecture - Wanted architecture
 * @param output_type - There are 2 types of files : "binary.zip" and "src.cpp"
 * @param output_file - Location of file to create
 * @param error - In case the export fails, the error is filled
 *
 * @return true if no error was encountered
 */
bool export_file(const std::string& url, const std::string& file, const std::string& os, const std::string& architecture, const std::string& output_type, const std::string& output_file, std::string& error);


//---------------------EXPORT DIVIDED IN 2 FUNCTION CALLS

/* Access to FaustWeb service - Post your faust file and get a corresponding SHA-Key
 *
 * @param url - FaustWeb server URL
 * @param file - Faust source file
 * @param key - Output SHA-Key
 * @param error - In case the export fails, the error is filled
 *
 * @return true if no error was encountered
 */
bool get_shaKey(const std::string& url, const std::string& file, std::string& key, std::string& error);


/* Access to FaustWeb service - Upload your faust application given the SHA-Key, an operating system and an architecture
 *
 * @param url - FaustWeb server URL
 * @param key - SHA-Key corresponding to your Faust code
 * @param os - Wanted Operating System
 * @param architecture - Wanted architecture
 * @param output_file - Location of file to create
 * @param error - In case the export fails, the error is filled
 *
 * @return true if no error was encountered
 */
bool get_file_from_key(const std::string& url, const std::string& key, const std::string& os, const std::string& architecture, const std::string& output_type, const std::string& output_file, std::string& error);



