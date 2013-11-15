//UTILITIES.cpp

#include "utilities.h"

//Returns the content of a file passed in path
string pathToContent(string path){
    
    QFile file(path.c_str());
    string Content;
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        //        printf("impossible to open content\n");
        Content = "";
        //        return Content;
    }
    while (!file.atEnd()) {
        //        printf("Content read\n");
        QByteArray line = file.readLine();
        Content += line.data();
    }
    
    //    printf("CONTENT = %s\n", Content.c_str());
    return Content;
}

//Delete recursively the content of a folder
bool deleteDirectoryAndContent(string& directory){
    
    QDir srcDir(directory.c_str());
    
    QFileInfoList children = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    
    if(children.size() == 0){
        QString toRemove(directory.c_str());
        srcDir.rmdir(srcDir.absolutePath());
        return true;
    }
    else{
        foreach(const QFileInfo &info, children) {
            
            QString path(directory.c_str());
            
            QString item = path + "/" + info.fileName();
            
            if (info.isDir()) {
                string file = item.toStdString();
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

QString searchLocalIP(){
    QRegExp httpPat ("[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}");
    QRegExp localPat ("127\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}");
    QRegExp bcastPat ("[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.255");
    QString result;
    
    int idx = 0;
    
    // Read network configuration ifconfig
    QProcess ppp;
    ppp.start( "/sbin/ifconfig");
    ppp.waitForFinished(-1);
    QString data = ppp.readAllStandardOutput();
    
    // Analyze network configuration to find IP number
    while ( (idx=data.indexOf(httpPat,idx)) != -1) {
        int n = httpPat.matchedLength();
        result = data.mid(idx, n);
        qDebug() << "check ip " << result;
        if ( (result.indexOf(localPat) == -1) && (result.indexOf(bcastPat) == -1) ) {
//            fIPaddress = result;
            qDebug() << "got ip " << result;
            return result;
        }
        idx += n;
    }
    
    qDebug() << "ip not found ! return localhost ";
    return "http://localhost";
}
