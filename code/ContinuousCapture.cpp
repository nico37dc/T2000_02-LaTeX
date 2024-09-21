#include "functions.hpp"

using namespace std;
using namespace mvIMPACT::acquire;
@\label{processRequest}@
//=============================================================================
//================= processRequest ============================================
//=============================================================================
void processRequest(shared_ptr<Request> pRequest, ThreadParameter &threadParameter, ConfigurationParameter &param, TimestampProvider &time, ImageSavePreparer &save, Logger &log, AzureIssueCreator &azureConnection)
{
  auto loggerClient = spdlog::get("logClient");
  ++threadParameter.requestsCaptured_;
  time.timeLastRequest = time.getNowTime();

  if (pRequest->isOK())
  {
    const unsigned int bufferPartCount = pRequest->getBufferPartCount();

    if (bufferPartCount > 0)  // Multi-part mode is running
    {
      cout << "Multipart buffer captured: " << bufferPartCount << " part" << ((bufferPartCount > 1) ? "s" : "") << endl;
      loggerClient->info("Multipart buffer captured: {} part{}", bufferPartCount, (bufferPartCount > 1 ? "s" : ""));

      for (unsigned int i = 0; i < bufferPartCount; i++)
      {
        // Display information about the captured buffer
        const ImageBuffer *pPart = pRequest->getBufferPart(i).getImageBufferDesc().getBuffer();
        cout << "Image " << pRequest->infoFrameID.read() << " part " << i << ": " << pPart->iWidth << "x" << pPart->iHeight << endl;
        loggerClient->info("Image {} part {}: {}x{}", pRequest->infoFrameID.read(), i, pPart->iWidth, pPart->iHeight);

        // Save images depending on the configuration
        if (param.saveImages && (param.saveImagesInterval == 1 || pRequest->infoFrameID.read() % param.saveImagesInterval == 1))
        {
          if (bufferPartCount == 1)  // Buffer has exactly one image
          {
            int imageNumber = ceil((float)pRequest->infoFrameID.read() / (float)param.saveImagesInterval);
            string filename = save.prepareImageSave(param, imageNumber);
            pRequest->getBufferPart(0).getImageBufferDesc().save(filename);
            loggerClient->info("Saved image {} to {}", pRequest->infoFrameID.read(), filename);
          }
          else // Buffer has more than one image
          {
            string filename = save.prepareBufferSave(param, pRequest->infoFrameID.read(), i);
            pRequest->getBufferPart(i).getImageBufferDesc().save(filename);
            loggerClient->info("Saved image {} part {} to {}", pRequest->infoFrameID.read(), i, filename);
          }
        }

        // Log timestamp information
        int64_t timestamp_us = pRequest->infoTimeStamp_us.read() / 1000;
        int64_t timestamp_ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
        loggerClient->info("Image: {}\t Timestamp_us: {}\t Timestamp: {}\t Difference: {}\n", pRequest->infoFrameID.read(), timestamp_us, timestamp_ms, timestamp_ms - timestamp_us);
      }
    }
    else  // Multi-part mode is not running
    {
      cout << "Error: RadarImager is not running in multi-part mode" << endl;
      loggerClient->error("Error: RadarImager is not running in multi-part mode\n");
      log.logStatistics(threadParameter);
      azureConnection.sendAzureIssue(param, threadParameter.statistics_.frameCount.read());
      exit(1);
    }
  }
  else  // Request error
  {
    cout << "Error: " << pRequest->requestResult.readS() << endl;
    loggerClient->error("{}\n", pRequest->requestResult.readS());
    log.logStatistics(threadParameter);
    azureConnection.sendAzureIssue(param, threadParameter.statistics_.frameCount.read());
    exit(1);
  }

  // Log some statistics and special data every few requests
  if (threadParameter.requestsCaptured_ % log.STATISTICS_INTERVAL == 0)
  {
    log.logStatistics(threadParameter);

    const Statistics &stats = threadParameter.statistics_;
    cout << "\nInfo from " << threadParameter.pDev_->serial.read()
       << ": " << stats.framesPerSecond.name() << ": " << stats.framesPerSecond.readS()
       << ", " << stats.errorCount.name() << ": " << stats.errorCount.readS()
       << ", " << stats.captureTime_s.name() << ": " << stats.captureTime_s.readS() << "\n" << endl;
  }
}

//=============================================================================
//================= main ======================================================
//=============================================================================
int main()
{
  ConfigurationParameter param;
  TimestampProvider time;
  ConfigurationHandler config;
  ImageSavePreparer save;
  Logger log;
  AzureIssueCreator azureConnection;

  config.configFilename = "../../configParameter.json";
  spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l] %v%$");
  auto loggerScript = spdlog::basic_logger_mt("logScript", "../Logfile.txt", true);
  loggerScript->set_level(spdlog::level::info);
  loggerScript->flush_on(spdlog::level::trace);

  if (!(config.loadConfiguration(param)))
  {
    cout << "Error: Reading config-file failed" << endl;
    loggerScript->info("Reading config-file failed");
    return 1;
  }

  log.initializeLogger(param);

  if (param.saveImages == false)
  {
    cout << "[images will not be saved]" << endl;
    loggerScript->info("[images will not be saved]");
  }
  else
  {
    cout << "[images will be saved - new folder every " << param.newFolderInterval << " images]" << endl;
    loggerScript->info("[images will be saved - new folder every {} images]", param.newFolderInterval);
  }

  DeviceManager devMgr;
  Device *pDev = getDeviceFromUserInput(devMgr);

  if (pDev == nullptr)
  {
    cout << "Unable to continue! Press [ENTER] to end the application" << endl;
    cin.get();
    return 1;
  }

  cout << "Initialising the device. This might take some time..." << endl;
  try
  {
    pDev->open();
  }
  catch (const ImpactAcquireException &e)
  {
    // this e.g. might happen if the same device is already opened in another process...
    cout << "An error occurred while opening the device " << pDev->serial.read() << "(error code: " << e.getErrorCodeAsString() << ").";
    loggerScript->error("An error occurred while opening the device {} (error code: {})", pDev->serial.read(), e.getErrorCodeAsString());
    return 1;
  }

  cout << "Press [ENTER] to stop the acquisition thread" << endl;

  ThreadParameter threadParameter(pDev);
  threadParameter.statistics_.reset();
  helper::RequestProvider requestProvider(pDev);
  time.timeLastRequest = time.getNowTime();
  requestProvider.acquisitionStart(processRequest, ref(threadParameter), ref(param), ref(time), ref(save), ref(log), ref(azureConnection));
  @\label{while-Schleife}@
  // exit the test if no more images are received
  while (!azureConnection.checkKeyboardHit())
  {
    if (time.getNowTime() - time.timeLastRequest > time.MAX_TIME_BETWEEN_IMAGES)
    {
      auto loggerClient = spdlog::get("logClient");
      cout << "Error: Maximum time between Images exceeded." << endl;
      loggerClient->error("Error: Maximum time between Images exceeded\n");
      log.logStatistics(threadParameter);
      azureConnection.sendAzureIssue(param, threadParameter.statistics_.frameCount.read());
      exit(1);
    }
    this_thread::sleep_for(chrono::milliseconds(1000));
  }

  requestProvider.acquisitionStop();
  log.logStatistics(threadParameter);
  return 0;
}