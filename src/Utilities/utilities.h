#ifndef _utilities_H
#define _utilities_H

#include <string>

#include <QString>
#include <QtWidgets>

#ifdef WIN32
#pragma warning (disable: 4100 4267)
#endif

void	writeFile(const QString& filePath, const QString& content);
QString readFile(const QString& filePath);
QString pathToContent(const QString& path);
void	touchFolder(const QString& path);
bool	executeInstruction(const QString& instruct, QString& errorMsg);
void	deleteDirectoryAndContent(const QString& directory);

bool	tarFolder(const QString& folder, QString& errorMsg);
bool	untarFolder(const QString& folder, QString& errorMsg);

bool	rmDir(const QString &dirPath);
bool	cpDir(const QString &srcPath, const QString &dstPath);

bool	isStringInt(const char* word);
QString searchLocalIP();

int		get_numberParameters(const QString& compilOptions);

std::string parse_compilationParams(QString& compilOptions);

bool parseKey(std::vector<std::string> options, const std::string& key, int& position);

bool addKeyIfExisting(std::vector<std::string>& options, std::vector<std::string>& newoptions, const std::string& key, const std::string& defaultKey, int& position);

void addKeyValueIfExisting(std::vector<std::string>& options, std::vector<std::string>& newoptions, const std::string& key, const std::string& defaultValue);

std::vector<std::string> reorganizeCompilationOptionsAux(std::vector<std::string>& options);

std::string FL_reorganize_compilation_options(QString compilationParams);
std::string FL_generate_sha1(const std::string& dsp_content);
void centerOnPrimaryScreen(QWidget* w);

#endif
