@\textcolor{KeywordPurple}{\#include}@ <cmath>
@\textcolor{KeywordPurple}{\#include}@ <chrono>
@\textcolor{KeywordPurple}{\#include}@ <string>
@\textcolor{KeywordPurple}{\#include}@ <thread>
@\textcolor{KeywordPurple}{\#include}@ <iomanip>
@\textcolor{KeywordPurple}{\#include}@ <sstream>
@\textcolor{KeywordPurple}{\#include}@ <fstream>
@\textcolor{KeywordPurple}{\#include}@ <fcntl.h>
@\textcolor{KeywordPurple}{\#include}@ <stdio.h>
@\textcolor{KeywordPurple}{\#include}@ <stdarg.h>
@\textcolor{KeywordPurple}{\#include}@ <unistd.h>
@\textcolor{KeywordPurple}{\#include}@ <iostream>
@\textcolor{KeywordPurple}{\#include}@ <termios.h>
@\textcolor{KeywordPurple}{\#include}@ <functional>
@\textcolor{KeywordPurple}{\#include}@ <filesystem>
@\textcolor{KeywordPurple}{\#include}@ <sys/stat.h>
@\textcolor{KeywordPurple}{\#include}@ <sys/types.h>

@\textcolor{KeywordPurple}{\#include}@ <apps/Common/exampleHelper.h>
@\textcolor{KeywordPurple}{\#include}@ <mvIMPACT_CPP/mvIMPACT_acquire_helper.h>

@\textcolor{KeywordPurple}{\#include}@ "@\textcolor{StringOrange}{base64.h}@"
@\textcolor{KeywordPurple}{\#include}@ "json.hpp"
@\textcolor{KeywordPurple}{\#include}@ "curl/curl.h"
@\textcolor{KeywordPurple}{\#include}@ "spdlog/spdlog.h"
@\textcolor{KeywordPurple}{\#include}@ "spdlog/sinks/ostream_sink.h"
@\textcolor{KeywordPurple}{\#include}@ "spdlog/sinks/basic_file_sink.h"
@\textcolor{KeywordPurple}{\#include}@ "spdlog/sinks/rotating_file_sink.h"

using namespace std;
@\label{ParamStruct}@
//=============================================================================
//================= ConfigurationParameter ====================================
//=============================================================================
struct ConfigurationParameter
{
  bool saveImages;
  bool numberImages;
  bool numberFolders;
  int saveImagesInterval;
  int newFolderInterval;
  bool createLog;
  int logLevel;
  int maxLogFiles;
  int maxLogSize;
  bool createIssue;
  string outputPath;
  string imagesPath;
  string logsPath;
  string testID;
};

//=============================================================================
//================= ThreadParameter ===========================================
//=============================================================================
struct ThreadParameter
{
  Device *pDev_;
  unsigned int requestsCaptured_;
  Statistics statistics_;
  explicit @\textcolor{FunctionsYellow}{ThreadParameter}@(Device *pDev) : pDev_(pDev), requestsCaptured_(0), statistics_(pDev) {}
  @\textcolor{FunctionsYellow}{ThreadParameter}@(const ThreadParameter &src) = delete;
  ThreadParameter &@\textcolor{FunctionsYellow}{operator=}@(const ThreadParameter &rhs) = delete;
};
@\label{TimestampProvider1}@
//=============================================================================
//================= TimestampProvider =========================================
//=============================================================================
class TimestampProvider
{
public:
  string getTimestamp();
  string getTimestamp_ms();
  time_t getNowTime();
  time_t timeLastRequest;
  const int MAX_TIME_BETWEEN_IMAGES = 180;
};
@\label{ConfigurationHandler}@
//=============================================================================
//================= ConfigurationHandler ======================================
//=============================================================================
class ConfigurationHandler
{
public:
  bool loadConfiguration(ConfigurationParameter &config);
  string configFilename;

private:
  bool checkPath(const string &path);
  bool createFolder(const string &path);
  bool readConfigurationParameter(ConfigurationParameter &config);
  bool checkConfigurationParameter(const ConfigurationParameter &config);
  bool createOutputDirectories(ConfigurationParameter &config);
  const int MIN_SAVE_IMAGES_INTERVAL = 1;
  const int MAX_SAVE_IMAGES_INTERVAL = 1000;
  const int MIN_NEW_FOLDER_INTERVAL = 1;
  const int MAX_NEW_FOLDER_INTERVAL = 1000;
  const int MIN_LOG_LEVEL = 0;
  const int MAX_LOG_LEVEL = 3;
  const int MIN_MAX_LOG_FILES = 1;
  const int MAX_MAX_LOG_FILES = 1000;
  const int MIN_MAX_LOG_SIZE = 1;
  const int MAX_MAX_LOG_SIZE = 100;
};
@\label{ImageSavePreparer1}@
//=============================================================================
//================= ImageSavePreparer =========================================
//=============================================================================
class ImageSavePreparer
{
public:
  string prepareImageSave(const ConfigurationParameter &config, const int imageNumber);
  string prepareBufferSave(const ConfigurationParameter &config, const int imageNumber, const int i);

private:
  string createImageSubpathFolder(const ConfigurationParameter &param, const int imageNumber);
  string createBufferSubpathFolder(const ConfigurationParameter &param, const int imageNumber, const int i);
  string imageLastSubpath;
  string bufferLastSubpath;
};

//=============================================================================
//================= Logger ====================================================
//=============================================================================
class Logger
{
public:
  bool initializeLogger(const ConfigurationParameter &config);
  void logStatistics(ThreadParameter &threadParameter);
  const int STATISTICS_INTERVAL = 100;

private:
  bool setLogLevel(const ConfigurationParameter &config);
  void interpretStatistics(ThreadParameter &threadParameter);
  const int ONE_MEGABYTE = 1048576;
  const float FPS_CHANGE_THRESHOLD = 0.5;
  int lastRequestCaptured = 0;
  int lastAbortedRequestCount = 0;
  float lastFramesPerSecond = 0;
  int lastFramesIncompleteCount = 0;
  int lastLostImagesCount = 0;
  int lastRetransmitCount = 0;
  int lastTimedOutRequestCount = 0;
};
@\label{AzureIssueCreator1}@
//=============================================================================
//================= AzureIssueCreator =========================================
//=============================================================================
class AzureIssueCreator
{
public:
  void sendAzureIssue(const ConfigurationParameter &param, const int frameCount);
  bool checkKeyboardHit();

private:
  bool createAzureIssue(const string title, const string description);
  static size_t handleCurlResponse(void *contents, size_t size, size_t nmemb, void *userp);
  string organization = "your_organization";
  string project = "your_project";
  string user = "your_name";  // Enter your Name
  string pat = "your_pat";  // Enter your PAT
};