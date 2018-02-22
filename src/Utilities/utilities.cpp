//UTILITIES.cpp

#include "utilities.h"

//#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <iostream> 
#include <fstream>

#include <QtNetwork>
#include <QWidgetList>

void writeFile(const QString& filePath, const QString& content)
{
    QFile f(filePath); 
     
    if (f.open(QFile::WriteOnly | QIODevice::Truncate)) {
        QTextStream textWriting(&f);
        textWriting<<content;
        f.close();
    }
}

QString readFile(const QString& filePath)
{
    QString content("");
    QFile f(filePath);
    
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        content = QLatin1String(f.readAll());
        f.close();
    }
    
    return content;
}

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

void touchFolder(const QString& path)
{
    QString instruction = "touch " + path;
    QString error("");
    executeInstruction(instruction, error);
}

bool executeInstruction(const QString& instruct, QString& errorMsg)
{
    QProcess myCmd;
    QByteArray error;
    
    myCmd.start(instruct);
    myCmd.waitForFinished();
    error = myCmd.readAllStandardError();
    
    if (myCmd.readChannel() == QProcess::StandardError ) {
        errorMsg = error.data();
		return false;
	} else {
		return true;
	}
}

//Delete recursively the content of a folder
void deleteDirectoryAndContent(const QString& directory)
{
    QString rmInstruct("rm -r ");
    rmInstruct += directory;
    QString errorMsg("");
    executeInstruction(rmInstruct, errorMsg);
}

bool tarFolder(const QString& folder, QString& errorMsg)
{
    QString systemInstruct("tar cfv ");
    systemInstruct += folder + ".tar " + folder;
    return executeInstruction(systemInstruct, errorMsg);
}

bool untarFolder(const QString& folder, QString& errorMsg)
{
    QString systemInstruct("tar xfv ");
    systemInstruct += folder +" -C /";
    return executeInstruction(systemInstruct, errorMsg);
}

//Remove a directory
bool rmDir(const QString &dirPath)
{
    QDir dir(dirPath);
    
    if (!dir.exists()) {
        return true;
    }
        
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
bool cpDir(const QString &srcPath, const QString &dstPath)
{
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
bool isStringInt(const char* word)
{
    bool returning = true;
    for (size_t i = 0; i < strlen(word); i++) {
        if (!isdigit(word[i])) {
            returning = false;
            break;
        }
    }
    return returning;
}

//Search IP adress in ifconfig result
QString searchLocalIP()
{
    printf("Utilities... Search for IP\n");
    
    QList<QHostAddress> ipAdresses = QNetworkInterface::allAddresses();
    QList<QHostAddress>::iterator it;
    QString localhost("localhost"); 
    
    for(it = ipAdresses.begin(); it != ipAdresses.end(); it++){
		if ((*it).protocol() == QAbstractSocket::IPv4Protocol && (*it) != QHostAddress::LocalHost) {
			printf("HOST = %s at pos = %i\n", it->toString().toStdString().c_str(), it->toString().indexOf("169."));
			//Filter strange windows default adress 169.x.x.x
			if(it->toString().indexOf("169.") != 0)
	            return it->toString();
		} else if ((*it).protocol() == QAbstractSocket::IPv4Protocol && (*it) == QHostAddress::LocalHost) {
            localhost = it->toString();
        }
    }
    
    return localhost;
}

//Look for 'key' in 'options' and modify the parameter 'position' if found
bool parseKey(vector<string> options, const string& key, int& position)
{
    for (size_t i = 0; i < options.size(); i++){
        if (key == options[i]){
            position = i;
            return true;
        }
    }
    
    return false;
}

//Add 'key' if existing in 'options', otherwise add 'defaultKey' (if different from "")
//#return true if 'key' was added
bool addKeyIfExisting(vector<string>& options, vector<string>& newoptions, const string& key, const string& defaultKey, int& position)
{
    if (parseKey(options, key, position)) {        
        newoptions.push_back(options[position]);
        options.erase(options.begin()+position);
        position--;
        return true;
    } else if (defaultKey != "") {
        newoptions.push_back(defaultKey);
    }
    
    return false;
}

//Add 'key' & it's associated value if existing in 'options', otherwise add 'defaultValue' (if different from "")
void addKeyValueIfExisting(vector<string>& options, vector<string>& newoptions, const string& key, const string& defaultValue)
{
    int position = 0;
    
    if (addKeyIfExisting(options, newoptions, key, "", position)) {
        if (position+1 < int(options.size()) && options[position+1][0] != '-') {
            newoptions.push_back(options[position+1]);
            options.erase(options.begin()+position+1);
            position--;
        } else {
            newoptions.push_back(defaultValue);
        }
    }
}

/* Reorganizes the compilation options
 * Following the tree of compilation (Faust_Compilation_Options.pdf in distribution)
 */
vector<string> reorganizeCompilationOptionsAux(vector<string>& options)
{
    bool vectorize = false;
    int position = 0;
    
    vector<string> newoptions;
    
    //------STEP 1 - Single or Double ?
    addKeyIfExisting(options, newoptions, "-double", "-single", position);
    
    //------STEP 2 - Options Leading to -vec inclusion
    if (addKeyIfExisting(options, newoptions, "-sch", "", position)) {
        vectorize = true;
    }
    
    if (addKeyIfExisting(options, newoptions, "-omp", "", position)){
        vectorize = true;
        addKeyIfExisting(options, newoptions, "-pl", "", position);
    }
    
    if (vectorize) {
        newoptions.push_back("-vec");
    }
    
    //------STEP3 - Add options depending on -vec/-scal option
    if (vectorize || addKeyIfExisting(options, newoptions, "-vec", "", position)) {
        addKeyIfExisting(options, newoptions, "-dfs", "", position);
        addKeyIfExisting(options, newoptions, "-vls", "", position);
        addKeyIfExisting(options, newoptions, "-fun", "", position);
        addKeyIfExisting(options, newoptions, "-g", "", position);
        addKeyValueIfExisting(options, newoptions, "-vs", "32");
        addKeyValueIfExisting(options, newoptions, "-lv", "0");
    } else {
        addKeyIfExisting(options, newoptions, "-scal", "-scal", position);
        addKeyIfExisting(options, newoptions, "-inpl", "", position);
    }
    
    addKeyValueIfExisting(options, newoptions, "-mcd", "16");
    addKeyValueIfExisting(options, newoptions, "-cn", "");
    
    //------STEP4 - Add other types of Faust options
    /*
     addKeyIfExisting(options, newoptions, "-tg", "", position);
     addKeyIfExisting(options, newoptions, "-sg", "", position);
     addKeyIfExisting(options, newoptions, "-ps", "", position);    
     addKeyIfExisting(options, newoptions, "-svg", "", position);    
     
     if (addKeyIfExisting(options, newoptions, "-mdoc", "", position)) {
     addKeyValueIfExisting(options, newoptions, "-mdlang", "");
     addKeyValueIfExisting(options, newoptions, "-stripdoc", "");
     }
     
     addKeyIfExisting(options, newoptions, "-sd", "", position);
     addKeyValueIfExisting(options, newoptions, "-f", "25");
     addKeyValueIfExisting(options, newoptions, "-mns", "40"); 
     addKeyIfExisting(options, newoptions, "-sn", "", position);
     addKeyIfExisting(options, newoptions, "-xml", "", position);
     addKeyIfExisting(options, newoptions, "-blur", "", position);    
     addKeyIfExisting(options, newoptions, "-lb", "", position);
     addKeyIfExisting(options, newoptions, "-mb", "", position);
     addKeyIfExisting(options, newoptions, "-rb", "", position);    
     addKeyIfExisting(options, newoptions, "-lt", "", position);    
     addKeyValueIfExisting(options, newoptions, "-a", "");
     addKeyIfExisting(options, newoptions, "-i", "", position);
     addKeyValueIfExisting(options, newoptions, "-cn", "");    
     addKeyValueIfExisting(options, newoptions, "-t", "120");
     addKeyIfExisting(options, newoptions, "-time", "", position);
     addKeyValueIfExisting(options, newoptions, "-o", "");
     addKeyValueIfExisting(options, newoptions, "-lang", "cpp");
     addKeyIfExisting(options, newoptions, "-flist", "", position);
     addKeyValueIfExisting(options, newoptions, "-l", "");
     addKeyValueIfExisting(options, newoptions, "-O", "");
     
     //-------Add Other Options that are possibily passed to the compiler (-I, -blabla, ...)
     while (options.size() != 0) {
     if (options[0] != "faust") newoptions.push_back(options[0]); // "faust" first argument
     options.erase(options.begin());
     }
     */
    
    return newoptions;
}

string FL_reorganize_compilation_options(QString compilationOptions)
{
    int argc = get_numberParameters(compilationOptions);
    vector<string> res1;
    for (int i = 0; i < argc; i++) {
        res1.push_back(parse_compilationParams(compilationOptions));
    }
    
    vector<string> res2 = reorganizeCompilationOptionsAux(res1);
    string res3;
    string sep;
    for (size_t i = 0; i < res2.size(); i++) {
        res3 = res3 + sep + res2[i];
        sep = " ";
    }
    
    return "\"" + res3 + "\"";
}

// Code by: B-Con (http://b-con.us)
// Released under the GNU GPL
// MD5 Hash Digest implementation (little endian byte order)

// Signed variables are for wimps
#define uchar unsigned char
#define uint unsigned int

// DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it
#define ROTLEFT(a,b) ((a << b) | (a >> (32-b)))
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - c) ++b; a += c;

typedef struct {
    uchar data[64];
    uint datalen;
    uint bitlen[2];
    uint state[5];
    uint k[4];
} SHA1_CTX;

static void sha1_transform(SHA1_CTX* ctx, uchar data[])
{
    uint a,b,c,d,e,i,j,t,m[80];
    
    for (i = 0,j = 0; i < 16; ++i, j += 4) {
        m[i] = (data[j] << 24) + (data[j+1] << 16) + (data[j+2] << 8) + (data[j+3]);
    }
    for ( ; i < 80; ++i) {
        m[i] = (m[i-3] ^ m[i-8] ^ m[i-14] ^ m[i-16]);
        m[i] = (m[i] << 1) | (m[i] >> 31);
    }
    
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    
    for (i=0; i < 20; ++i) {
        t = ROTLEFT(a,5) + ((b & c) ^ (~b & d)) + e + ctx->k[0] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b,30);
        b = a;
        a = t;
    }
    for ( ; i < 40; ++i) {
        t = ROTLEFT(a,5) + (b ^ c ^ d) + e + ctx->k[1] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b,30);
        b = a;
        a = t;
    }
    for ( ; i < 60; ++i) {
        t = ROTLEFT(a,5) + ((b & c) ^ (b & d) ^ (c & d))  + e + ctx->k[2] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b,30);
        b = a;
        a = t;
    }
    for ( ; i < 80; ++i) {
        t = ROTLEFT(a,5) + (b ^ c ^ d) + e + ctx->k[3] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b,30);
        b = a;
        a = t;
    }
    
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
}

static void sha1_init(SHA1_CTX* ctx)
{
    ctx->datalen = 0;
    ctx->bitlen[0] = 0;
    ctx->bitlen[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
    ctx->k[0] = 0x5a827999;
    ctx->k[1] = 0x6ed9eba1;
    ctx->k[2] = 0x8f1bbcdc;
    ctx->k[3] = 0xca62c1d6;
}

static void sha1_update(SHA1_CTX* ctx, uchar data[], uint len)
{
    uint i;
    
    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha1_transform(ctx, ctx->data);
            DBL_INT_ADD(ctx->bitlen[0], ctx->bitlen[1], 512);
            ctx->datalen = 0;
        }
    }
}

static void sha1_final(SHA1_CTX *ctx, uchar hash[])
{
    uint i;
    
    i = ctx->datalen;
    
    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) {
            ctx->data[i++] = 0x00;
        }
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) {
            ctx->data[i++] = 0x00;
        }
        sha1_transform(ctx, ctx->data);
        memset(ctx->data,0,56);
    }
    
    // Append to the padding the total message's length in bits and transform.
    DBL_INT_ADD(ctx->bitlen[0], ctx->bitlen[1], 8 * ctx->datalen);
    ctx->data[63] = ctx->bitlen[0];
    ctx->data[62] = ctx->bitlen[0] >> 8;
    ctx->data[61] = ctx->bitlen[0] >> 16;
    ctx->data[60] = ctx->bitlen[0] >> 24;
    ctx->data[59] = ctx->bitlen[1];
    ctx->data[58] = ctx->bitlen[1] >> 8;
    ctx->data[57] = ctx->bitlen[1] >> 16;
    ctx->data[56] = ctx->bitlen[1] >> 24;
    sha1_transform(ctx,ctx->data);
    
    // Since this implementation uses little endian byte ordering and MD uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]    = (ctx->state[0] >> (24-i*8)) & 0x000000ff;
        hash[i+4]  = (ctx->state[1] >> (24-i*8)) & 0x000000ff;
        hash[i+8]  = (ctx->state[2] >> (24-i*8)) & 0x000000ff;
        hash[i+12] = (ctx->state[3] >> (24-i*8)) & 0x000000ff;
        hash[i+16] = (ctx->state[4] >> (24-i*8)) & 0x000000ff;
    }
}

string FL_generate_sha1(const string& dsp_content)
{
    SHA1_CTX ctx;
    unsigned char obuf[20] = {0};
    
    // Hash one
    sha1_init(&ctx);
    sha1_update(&ctx, (unsigned char*)dsp_content.c_str(), dsp_content.size());
    sha1_final(&ctx, obuf);
  
	// convert SHA1 key into hexadecimal string
    string sha1key;
    for (int i = 0; i < 20; i++) {
    	const char* H = "0123456789ABCDEF";
    	char c1 = H[(obuf[i] >> 4)];
    	char c2 = H[(obuf[i] & 15)];
        sha1key += c1;
        sha1key += c2;
    }
    
    return sha1key;
}

//---------------COMPILATION OPTIONS

//Get number of compilation options
int get_numberParameters(const QString& compilOptions)
{
    int argc = 0;
    int pos = compilOptions.indexOf("-");
    
    while (pos != -1 && pos < compilOptions.size()) {
        
        argc++;
        pos = compilOptions.indexOf(" ", pos);
        
        while (pos != -1 && pos < compilOptions.size()) {
            if (compilOptions[pos] == ' ') {
                pos++;
            } else if(compilOptions[pos] == '-') {
                break;
            } else {
                argc++;
                pos = compilOptions.indexOf(" ", pos);
            }
        }
    }
    
    return argc;
}

//Hand written Parser
//Returns : the first option found, skipping the ' '
//CompilOptions : the rest of the options are kept in
string parse_compilationParams(QString& compilOptions)
{
    QString returning = "";
    int pos = 0;
    
    while (pos != -1 && pos < compilOptions.size()) {
        if (compilOptions[pos] == ' ') {
            pos++;
        } else {
            int pos2 = compilOptions.indexOf(" ", pos);
            returning = compilOptions.mid(pos, pos2-pos);
            if (pos2 == -1) {
                pos2 = compilOptions.size();
            }
            compilOptions.remove(0, pos2);
            break;
        }
    }
    
    return returning.toStdString();
}

// center a widget on the primary screen of the machine or 
// on the screen of the top level widget
void centerOnPrimaryScreen(QWidget* w)
{
    QDesktopWidget *dw = QApplication::desktop();
	QWidgetList l = QApplication::topLevelWidgets();
    
	if (l.empty()) {
    	w->move(dw->availableGeometry(dw->primaryScreen()).center() - w->geometry().center());
    } else {
    	QWidget* topwidget = l.first();	
    	w->move(dw->screenGeometry(topwidget).center() - w->geometry().center());
    }
}


