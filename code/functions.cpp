#include "functions.hpp"

using namespace std;
@\label{TimestampProvider2}@
//=============================================================================
//================= getNowTime ================================================
//=============================================================================
time_t TimestampProvider::getNowTime()
{
  time_t nowTime;
  auto @\textcolor{black}{now}@ = chrono::system_clock::now();
  nowTime = chrono::system_clock::to_time_t(@\textcolor{black}{now}@);
  return nowTime;
}

//=============================================================================
//================= getTimestamp ==============================================
//=============================================================================
string TimestampProvider::getTimestamp()
{
  time_t nowTime = getNowTime();

  stringstream ss;
  ss << put_time(localtime(&nowTime), "%Y-%m-%d_%Hh-%Mm-%Ss");
  return ss.str();
}

//=============================================================================
//================= getTimestamp_ms ===========================================
//=============================================================================
string TimestampProvider::getTimestamp_ms()
{
  time_t nowTime = getNowTime();
  auto now = chrono::system_clock::now();
  auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

  stringstream ss;
  ss << put_time(localtime(&nowTime), "%Y-%m-%d_%Hh-%Mm-%Ss-") << setfill('0') << setw(3) << ms.count() << "ms";
  return ss.str();
}

//=============================================================================
//================= readConfigurationParameter ================================
//=============================================================================
bool ConfigurationHandler::readConfigurationParameter(ConfigurationParameter &param)
{
  auto loggerScript = spdlog::get("logScript");
  ifstream paramFile(configFilename);
  nlohmann::json root;
  paramFile >> root;

  // Check if the parameters in the json-file has the correct datatype
  if (!root["Basic"]["testID"].is_string() ||
    !root["GenICam-Client"]["saveImages"].is_boolean() ||
    !root["GenICam-Client"]["numberImages"].is_boolean() ||
    !root["GenICam-Client"]["numberFolders"].is_boolean() ||
    !root["GenICam-Client"]["saveImagesInterval"].is_number_integer() ||
    !root["GenICam-Client"]["newFolderInterval"].is_number_integer() ||
    !root["GenICam-Client"]["createLog"].is_boolean() ||
    !root["GenICam-Client"]["logLevel"].is_number_integer() ||
    !root["GenICam-Client"]["maxLogFiles"].is_number_integer() ||
    !root["GenICam-Client"]["maxLogSize"].is_number_integer() ||
    !root["GenICam-Client"]["createIssue"].is_boolean() ||
    !root["GenICam-Client"]["outputPath"].is_string())
  {
    loggerScript->error("Error: incompatible data type of the parameters.");
    cout << "Error: incompatible data type of the parameters." << endl;
    return false;
  }

  // Copy the parameters into variables
  try
  {
    param.testID = root["Basic"]["testID"];
    param.saveImages = root["GenICam-Client"]["saveImages"];
    param.numberImages = root["GenICam-Client"]["numberImages"];
    param.numberFolders = root["GenICam-Client"]["numberFolders"];
    param.saveImagesInterval = root["GenICam-Client"]["saveImagesInterval"];
    param.newFolderInterval = root["GenICam-Client"]["newFolderInterval"];
    param.createLog = root["GenICam-Client"]["createLog"];
    param.logLevel = root["GenICam-Client"]["logLevel"];
    param.maxLogFiles = root["GenICam-Client"]["maxLogFiles"];
    param.maxLogSize = root["GenICam-Client"]["maxLogSize"];
    param.createIssue = root["GenICam-Client"]["createIssue"];
    param.outputPath = root["GenICam-Client"]["outputPath"];
  }
  catch (const exception &e)
  {
    loggerScript->error("Error: loading the parameters failed.");
    cout << "Error: loading the parameters failed." << endl;
    return false;
  }

  return true;
}

//=============================================================================
//================= checkConfigurationParameter ===============================
//=============================================================================
// Check if the parameters are within the specified ranges or have the correct format
bool ConfigurationHandler::checkConfigurationParameter(const ConfigurationParameter &param)
{
  auto loggerScript = spdlog::get("logScript");

  if (param.saveImagesInterval < MIN_SAVE_IMAGES_INTERVAL || param.saveImagesInterval > MAX_SAVE_IMAGES_INTERVAL)
  {
    loggerScript->error("Error: saveImagesInterval invalid.");
    cout << "Error: saveImagesInterval invalid." << endl;
    return false;
  }

  if (param.newFolderInterval < MIN_NEW_FOLDER_INTERVAL || param.newFolderInterval > MAX_NEW_FOLDER_INTERVAL)
  {
    loggerScript->error("Error: newFolderInterval invalid.");
    cout << "Error: newFolderInterval invalid." << endl;
    return false;
  }

  if (param.logLevel < MIN_LOG_LEVEL || param.logLevel > MAX_LOG_LEVEL)
  {
    loggerScript->error("Error: logLevel invalid.");
    cout << "Error: logLevel invalid." << endl;
    return false;
  }

  if (param.maxLogFiles < MIN_MAX_LOG_FILES || param.maxLogFiles > MAX_MAX_LOG_FILES)
  {
    loggerScript->error("Error: maxLogFiles invalid.");
    cout << "Error: maxLogFiles invalid." << endl;
    return false;
  }

  if (param.maxLogSize < MIN_MAX_LOG_SIZE || param.maxLogSize > MAX_MAX_LOG_SIZE)
  {
    loggerScript->error("Error: maxLogSize invalid.");
    cout << "Error: maxLogSize invalid." << endl;
    return false;
  }

  if (param.testID.empty())
  {
    loggerScript->error("Error: testID empty.");
    cout << "Error: testID empty." << endl;
    return false;
  }

  if (!checkPath(param.outputPath))
  {
    loggerScript->error("Error: output path invalid.");
    cout << "Error: output path invalid." << endl;
    return false;
  }

  return true;
}

//=============================================================================
//================= checkPath =================================================
//=============================================================================
bool ConfigurationHandler::checkPath(const string &path)
{
  auto loggerScript = spdlog::get("logScript");

  if (path.empty())
  {
    loggerScript->error("Error: empty output path.");
    cout << "Error: empty output path." << endl;
    return false;
  }

  for (char c : path)
  {
    if (!(isalnum(c) || c == '/' || c == '.' || c == '-' || c == '_' || c == '~'))
    {
      loggerScript->error("Error: invalid character in output path.");
      cout << "Error: invalid character in output path." << endl;
      return false;
    }
  }

  return true;
}

//=============================================================================
//================= createFolder ==============================================
//=============================================================================
bool ConfigurationHandler::createFolder(const string &path)
{
  auto loggerScript = spdlog::get("logScript");

  if (!(filesystem::create_directories(path)))
  {
    loggerScript->error("Error: creating the folder failed.");
    cout << "Error: creating the folder failed." << endl;
    return false;
  }

  return true;
}

//=============================================================================
//================= createOutputDirectories ===================================
//=============================================================================
bool ConfigurationHandler::createOutputDirectories(ConfigurationParameter &param)
{
  auto loggerScript = spdlog::get("logScript");

  TimestampProvider time;
  string timestamp = time.getTimestamp();

  param.outputPath = param.outputPath + "/Output_" + param.testID + "_" + timestamp;
  param.imagesPath = param.outputPath + "/Images_" + param.testID + "_" + timestamp;
  param.logsPath = param.outputPath + "/Logs_" + param.testID + "_" + timestamp;

  if (!createFolder(param.outputPath) ||
      !createFolder(param.imagesPath) ||
      !createFolder(param.logsPath))
  {
    loggerScript->error("Error: creating the output directories failed.");
    cout << "Error: creating the output directories failed." << endl;
    return false;
  }

  return true;
}

//=============================================================================
//================= loadConfiguration =========================================
//=============================================================================
bool ConfigurationHandler::loadConfiguration(ConfigurationParameter &param)
{
  auto loggerScript = spdlog::get("logScript");

  if (!readConfigurationParameter(param) ||
      !checkConfigurationParameter(param) ||
      !createOutputDirectories(param))
  {
    loggerScript->error("Error: loading the parameters failed.");
    cout << "Error: loading the parameters failed." << endl;
    return false;
  }

  return true;
}
@\label{ImageSavePreparer2}@
//=============================================================================
//================= createImageSubpathFolder ==================================
//=============================================================================
string ImageSavePreparer::createImageSubpathFolder(const ConfigurationParameter &param, const int imageNumber)
{
  TimestampProvider time;
  string subpath;

  if (param.numberFolders)
  {
    int folderNo = ceil((float)imageNumber / (float)param.newFolderInterval);
    subpath = param.imagesPath + "/" + to_string(folderNo) + "/";
    if (imageNumber % param.newFolderInterval == 1)
    {
      mkdir(subpath.c_str(), 0777);
    }
  }
  else
  {
    if (imageNumber % param.newFolderInterval == 1)
    {
      string timestamp_ms = time.getTimestamp_ms();
      subpath = param.imagesPath + "/" + timestamp_ms + "/";
      mkdir(subpath.c_str(), 0777);
      imageLastSubpath = subpath;
    }
    else
    {
      subpath = imageLastSubpath;
    }
  }

  return subpath;
}

//=============================================================================
//================= prepareImageSave ==========================================
//=============================================================================
string ImageSavePreparer::prepareImageSave(const ConfigurationParameter &param, const int imageNumber)
{
  TimestampProvider time;
  string filename;
  string subpath = createImageSubpathFolder(param, imageNumber);

  if (param.numberImages)
  {
    filename = subpath + "image_" + to_string(imageNumber) + ".png";
  }
  else
  {
    string timestamp_ms = time.getTimestamp_ms();
    filename = subpath + "image_" + timestamp_ms + ".png";
  }

  return filename;
}

//=============================================================================
//================= createBufferSubpathFolder =================================
//=============================================================================
string ImageSavePreparer::createBufferSubpathFolder(const ConfigurationParameter &param, const int imageNumber, const int i)
{
  TimestampProvider time;
  string subpath;

  if (param.numberFolders)
  {
    subpath = param.imagesPath + "/" + to_string(imageNumber) + "/";
    if (i == 0)
    {
      mkdir(subpath.c_str(), 0777);
    }
  }
  else
  {
    if (i == 0)
    {
      subpath = param.imagesPath + "/" + time.getTimestamp_ms() + "/";
      bufferLastSubpath = subpath;
    }
    else
    {
      subpath = bufferLastSubpath;
    }
  }

  return subpath;
}
@\label{prepareBufferSave}@
//=============================================================================
//================= prepareBufferSave  ========================================
//=============================================================================
string ImageSavePreparer::prepareBufferSave(const ConfigurationParameter &param, const int imageNumber, const int i)
{
  TimestampProvider time;
  string filename;
  string subpath = createBufferSubpathFolder(param, imageNumber, i);

  if (param.numberImages)
  {
    filename = subpath + "image_" + to_string(imageNumber) + "part." + to_string(i) + ".png";
  }
  else
  {
    filename = subpath + "image_" + to_string(imageNumber) + "_" + time.getTimestamp_ms() + ".png";
  }

  return filename;
}
@\label{setLogLevel}@
//=============================================================================
//================= setLogLevel ===============================================
//=============================================================================
bool Logger::setLogLevel(const ConfigurationParameter &param)
{
  auto loggerClient = spdlog::get("logClient");
  auto loggerScript = spdlog::get("logScript");

  if (param.createLog)
  {
    switch (param.logLevel)
    {
    case 0:
      loggerClient->set_level(spdlog::level::off);
      break;
    case 1:
      loggerClient->set_level(spdlog::level::err);
      break;
    case 2:
      loggerClient->set_level(spdlog::level::warn);
      break;
    case 3:
      loggerClient->set_level(spdlog::level::info);
      break;
    default:
      loggerScript->error("Error: invalid Log-Level");
      cerr << "Error: invalid Log-Level" << endl;
      return false;
      break;
    }
  }
  else
  {
    loggerClient->set_level(spdlog::level::off);
  }

  return true;
}
@\label{initializeLogger}@
//=============================================================================
//================= initializeLogger ==========================================
//=============================================================================
bool Logger::initializeLogger(const ConfigurationParameter &param)
{
  string logFileName = param.logsPath + "/Logfile.txt";

  auto loggerClient = spdlog::rotating_logger_mt("logClient", logFileName, (ONE_MEGABYTE * param.maxLogSize), (param.maxLogFiles - 1));
  setLogLevel(param);
  loggerClient->flush_on(spdlog::level::trace);

  return true;
}

//=============================================================================
//================= logStatistics =============================================
//=============================================================================
void Logger::logStatistics(ThreadParameter &threadParameter)
{
  auto loggerClient = spdlog::get("logClient");

  const Statistics &s = threadParameter.statistics_;
  loggerClient->info("Info from {}:", threadParameter.pDev_->serial.read());
  loggerClient->info("{}: {}", s.abortedRequestsCount.name(), s.abortedRequestsCount.readS());
  loggerClient->info("{}: {}", s.bandwidthConsumed.name(), s.bandwidthConsumed.readS());
  loggerClient->info("{}: {}", s.captureTime_s.name(), s.captureTime_s.readS());
  loggerClient->info("{}: {}", s.errorCount.name(), s.errorCount.readS());
  loggerClient->info("{}: {}", s.formatConvertTime_s.name(), s.formatConvertTime_s.readS());
  loggerClient->info("{}: {}", s.frameCount.name(), s.frameCount.readS());
  loggerClient->info("{}: {}", s.framesIncompleteCount.name(), s.framesIncompleteCount.readS());
  loggerClient->info("{}: {}", s.framesPerSecond.name(), s.framesPerSecond.readS());
  loggerClient->info("{}: {}", s.imageProcTime_s.name(), s.imageProcTime_s.readS());
  loggerClient->info("{}: {}", s.lostImagesCount.name(), s.lostImagesCount.readS());
  loggerClient->info("{}: {}", s.missingDataAverage_pc.name(), s.missingDataAverage_pc.readS());
  loggerClient->info("{}: {}", s.queueTime_s.name(), s.queueTime_s.readS());
  loggerClient->info("{}: {}", s.retransmitCount.name(), s.retransmitCount.readS());
  loggerClient->info("{}: {}\n", s.timedOutRequestsCount.name(), s.timedOutRequestsCount.readS());

  interpretStatistics(threadParameter);
}

//=============================================================================
//================= interpretStatistics =======================================
//=============================================================================
// Log warnings about significant changes or special values of the statistics
void Logger::interpretStatistics(ThreadParameter &threadParameter)
{
  auto loggerClient = spdlog::get("logClient");

  const Statistics &s = threadParameter.statistics_;
  bool anyWarningLogged = false;

  int requestDifference = threadParameter.requestsCaptured_ - lastRequestCaptured;

  if (s.abortedRequestsCount.read() > lastAbortedRequestCount)
  {
    loggerClient->warn("There were {} requests aborted over the last {} images", s.abortedRequestsCount.read() - lastAbortedRequestCount, requestDifference);
    anyWarningLogged = true;
  }

  if (abs(s.framesPerSecond.read() - lastFramesPerSecond) > FPS_CHANGE_THRESHOLD * max(lastFramesPerSecond, 1.0f))
  {
    loggerClient->warn("The Frames per Second changed by more than 50% of the last value. Current: {}, Last: {}", s.framesPerSecond.read(), lastFramesPerSecond);
    anyWarningLogged = true;
  }

  if (s.framesIncompleteCount.read() > lastFramesIncompleteCount)
  {
    loggerClient->warn("There were {} frames incomplete over the last {} images", s.framesIncompleteCount.read() - lastFramesIncompleteCount, requestDifference);
    anyWarningLogged = true;
  }

  if (s.lostImagesCount.read() > lastLostImagesCount)
  {
    loggerClient->warn("There were {} images lost over the last {} images", s.lostImagesCount.read() - lastLostImagesCount, requestDifference);
    anyWarningLogged = true;
  }

  if (s.retransmitCount.read() > lastRetransmitCount)
  {
    loggerClient->warn("There were {} requests retransmitted over the last {} images", s.retransmitCount.read() - lastRetransmitCount, requestDifference);
    anyWarningLogged = true;
  }

  if (s.timedOutRequestsCount.read() > lastTimedOutRequestCount)
  {
    loggerClient->warn("There were {} requests timed out over the last {} images", s.timedOutRequestsCount.read() - lastTimedOutRequestCount, requestDifference);
    anyWarningLogged = true;
  }

  if (anyWarningLogged)
  {
    loggerClient->info("These were the problematic statistics\n");
  }
  else
  {
    loggerClient->info("The Statistics are within acceptable limits\n");
  }

  lastRequestCaptured = threadParameter.requestsCaptured_;
  lastAbortedRequestCount = s.abortedRequestsCount.read();
  lastFramesPerSecond = s.framesPerSecond.read();
  lastFramesIncompleteCount = s.framesIncompleteCount.read();
  lastRetransmitCount = s.retransmitCount.read();
  lastTimedOutRequestCount = s.timedOutRequestsCount.read();
}
@\label{AzureIssueCreator2}@
//=============================================================================
//================= handleCurlResponse ========================================
//=============================================================================
size_t AzureIssueCreator::handleCurlResponse(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

//=============================================================================
//================= createAzureIssue ==========================================
//=============================================================================
bool AzureIssueCreator::createAzureIssue(const string title, const string description)
{
  auto loggerScript = spdlog::get("logScript");

  CURL *curl;
  CURLcode res;
  string readBuffer;

  string jsonData = R@\textcolor{StringOrange}{}@(
  [
    {
      "op": "add",
      "path": "/fields/System.Title",
      "value": ")" + title +
               R"(",
    },
    {
      "op": "add",
      "path": "/fields/System.Description",
      "value": ")" + description +
               R"(",
    },
    {
      "op": "add",
      "path": "/fields/System.AreaPath",
      "value": "SIP DohO Imaging Radar\\Software",
    },
    {
      "op": "add",
      "path": "/fields/System.AssignedTo",
      "value": ")" + user +
               R"(",
    },
  ])@\textcolor{StringOrange}{}@;

  string url = "https://dev.azure.com/" + organization + "/" + project + "/_apis/wit/workitems/$Issue?api-version=6.0";

  string encoded_pat = base64_encode(":" + pat);

  curl = curl_easy_init();
  if (curl)
  {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json-patch+json");

    string authHeader = "Authorization: Basic " + encoded_pat;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleCurlResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
      loggerScript->error("cURL Error: {}", curl_easy_strerror(res));
      cerr << "cURL Error: " << curl_easy_strerror(res) << endl;
      return false;
    }
    else
    {
      loggerScript->info("cURL Response from server: {}", readBuffer);
      cout << "cURL Response from server:" << readBuffer << endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  return true;
}

//=============================================================================
//================= sendAzureIssue ============================================
//=============================================================================
void AzureIssueCreator::sendAzureIssue(const ConfigurationParameter &param, const int frameCount)
{
  TimestampProvider time;
  auto loggerClient = spdlog::get("logClient");

  if (param.createIssue)
  {
    string title;
    title = "Test: " + param.testID + " failed!";
    string description;
    description = "Test failed / Time: " + time.getTimestamp() + " / Images: " + to_string(frameCount);

    if (createAzureIssue(title, description))
    {
      loggerClient->info("\n AzureIssue created: Title: {}, Description: {}.\n", title, description);
    }
    else
    {
      loggerClient->info("\n Creating AzureIssue failed\n");
    }
    
  }
}
@\label{checkKeyboardHit}@
//=============================================================================
//================= checkKeyboardHit ==========================================
//=============================================================================
bool AzureIssueCreator::checkKeyboardHit()
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return true;
  }

  return false;
}