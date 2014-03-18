//UTILITIES.cpp

#include "utilities.h"

//#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <iostream> 
#include <fstream>

#include <QtNetwork>

////Returns the content of a file passed in path
//QString pathToContent(QString path){
//    
//    QFile file(path);
//    QString Content;
//    
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
//        //        printf("impossible to open content\n");
//        Content = "";
//        //        return Content;
//    }
//    while (!file.atEnd()) {
//        //        printf("Content read\n");
//        QByteArray line = file.readLine();
//        Content += line.data();
//    }
//    
//    //    printf("CONTENT = %s\n", Content.c_str());
//    return Content;
//}

//Returns the content of a file passed in path
QString pathToContent(const QString& path)
{
    ifstream file(path.toStdString().c_str(), std::ifstream::binary);
    
    file.seekg (0, file.end);
    int size = file.tellg();
    file.seekg (0, file.beg);
    
    // And allocate buffer to that a single line can be read...
    char* buffer = new char[size + 1];
    file.read(buffer, size);
    
    // Terminate the string
    buffer[size] = 0;
    QString result = buffer;
    file.close();
    delete [] buffer;
    return result;
}


//Delete recursively the content of a folder
bool deleteDirectoryAndContent(QString& directory){
    
    QDir srcDir(directory);
    
    QFileInfoList children = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
    if(children.size() == 0){
        QString toRemove(directory);
        srcDir.rmdir(srcDir.absolutePath());
        return true;
    }
    else{
        foreach(const QFileInfo &info, children) {
            
            QString path(directory);
            
            QString item = path + "/" + info.fileName();
            
            if (info.isDir()) {
                QString file = item;
                if (!deleteDirectoryAndContent(file)) {
                    return false;
                }
            } else if (info.isFile()) {
                if (!QFile::remove(item)) {
                    return false;
                }
            } else {
                qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
            }
        }
        return true;
    }
}

//Remove a directory
bool rmDir(const QString &dirPath){
    QDir dir(dirPath);
    if (!dir.exists())
        return true;
    foreach(const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (info.isDir()) {
            if (!rmDir(info.filePath()))
                return false;
        } else {
            if (!dir.remove(info.fileName()))
                return false;
        }
    }
    QDir parentDir(QFileInfo(dirPath).path());
    return parentDir.rmdir(QFileInfo(dirPath).fileName());
}

//Copy a directory
bool cpDir(const QString &srcPath, const QString &dstPath){
    rmDir(dstPath);
    QDir parentDstDir(QFileInfo(dstPath).path());
    if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
        return false;
    
    QDir srcDir(srcPath);
    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir()) {
            if (!cpDir(srcItemPath, dstItemPath)) {
                return false;
            }
        } else if (info.isFile()) {
            if (!QFile::copy(srcItemPath, dstItemPath)) {
                return false;
            }
        } else {
            qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
        }
    }
    return true;
}

//Verify if the word is a number
bool isStringInt(const char* word){
    
    bool returning = true;
    
    for(size_t i=0; i<strlen(word); i++){
        if(!isdigit(word[i])){
            returning = false;
            break;
        }
    }
    return returning;
}



//Search IP adress in ifconfig result
QString searchLocalIP(){
    
    printf("Utilities... Search for IP\n");
    
    QList<QHostAddress> ipAdresses = QNetworkInterface::allAddresses();
    
    QList<QHostAddress>::iterator it;
    
    QString localhost("localhost"); 
    
    for(it = ipAdresses.begin(); it != ipAdresses.end(); it++){
        if((*it).protocol() == QAbstractSocket::IPv4Protocol && (*it) != QHostAddress::LocalHost)
            return it->toString();
        else if((*it).protocol() == QAbstractSocket::IPv4Protocol && (*it) == QHostAddress::LocalHost)
            localhost = it->toString();
    }
    
    return localhost;
}

const char* lopts(char *argv[], const char *name, const char* def)
{
	int	i;
	for (i = 0; argv[i]; i++) if (!strcmp(argv[i], name)) return argv[i+1];
	return def;
}
