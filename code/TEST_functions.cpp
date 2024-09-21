#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "functions.hpp"

using ::testing::_;
using ::testing::Return;

//=============================================================================
//================= TimestampProviderTest =====================================
//=============================================================================
class TimestampProviderTest : public ::testing::Test
{
protected:
  TimestampProvider time;
};

TEST_F(TimestampProviderTest, getNowTime)
{
  std::time_t nowTime = time.getNowTime();
  std::time_t currentTime = std::time(nullptr);
  int absTolerance = 1;
  EXPECT_NEAR(nowTime, currentTime, absTolerance);
}

TEST_F(TimestampProviderTest, getTimestamp)
{
  std::string timestamp = time.getTimestamp();
  EXPECT_EQ(timestamp.length(), 22);
  EXPECT_EQ(timestamp[4], '-');
  EXPECT_EQ(timestamp[10], '_');
  EXPECT_EQ(timestamp[13], 'h');
  EXPECT_EQ(timestamp[21], 's');
}

TEST_F(TimestampProviderTest, getTimestamp_ms)
{
  std::string timestamp_ms = time.getTimestamp_ms();
  EXPECT_EQ(timestamp_ms.length(), 28);
  EXPECT_EQ(timestamp_ms[4], '-');
  EXPECT_EQ(timestamp_ms[10], '_');
  EXPECT_EQ(timestamp_ms[13], 'h');
  EXPECT_EQ(timestamp_ms[22], '-');
  EXPECT_EQ(timestamp_ms[27], 's');
}

//=============================================================================
//================= ConfigurationHandlerTest ==================================
//=============================================================================
class ConfigurationHandlerTest : public ::testing::Test
{
protected:
  ConfigurationHandler config;
  ConfigurationParameter param;

  void SetUp() override
  {
    auto loggerScript = spdlog::basic_logger_mt("logScript", "/tmp/Logfile.txt");
    loggerScript->set_level(spdlog::level::info);
    loggerScript->flush_on(spdlog::level::trace);
  }

  void TearDown() override
  {
    spdlog::drop("logScript");
  }
};

TEST_F(ConfigurationHandlerTest, ValidConfigA)
{
  config.configFilename = "../test/TEST1_configParameter.json";
  EXPECT_TRUE(config.loadConfiguration(param));

  EXPECT_EQ(param.saveImages, true);
  EXPECT_EQ(param.numberImages, true);
  EXPECT_EQ(param.numberFolders, true);
  EXPECT_EQ(param.saveImagesInterval, 1);
  EXPECT_EQ(param.newFolderInterval, 10);
  EXPECT_EQ(param.createLog, true);
  EXPECT_EQ(param.logLevel, 1);
  EXPECT_EQ(param.maxLogFiles, 3);
  EXPECT_EQ(param.maxLogSize, 5);
  EXPECT_EQ(param.createIssue, true);
  EXPECT_EQ(param.testID, "TestID");

  EXPECT_TRUE(std::filesystem::exists(param.outputPath));
  EXPECT_TRUE(std::filesystem::exists(param.imagesPath));
  EXPECT_TRUE(std::filesystem::exists(param.logsPath));

  std::filesystem::remove_all(param.outputPath);
}

TEST_F(ConfigurationHandlerTest, ValidConfigB)
{
  config.configFilename = "../test/TEST2_configParameter.json";
  EXPECT_TRUE(config.loadConfiguration(param));

  EXPECT_EQ(param.saveImages, false);
  EXPECT_EQ(param.numberImages, false);
  EXPECT_EQ(param.numberFolders, false);
  EXPECT_EQ(param.saveImagesInterval, 100);
  EXPECT_EQ(param.newFolderInterval, 1);
  EXPECT_EQ(param.createLog, true);
  EXPECT_EQ(param.logLevel, 3);
  EXPECT_EQ(param.maxLogFiles, 3);
  EXPECT_EQ(param.maxLogSize, 5);
  EXPECT_EQ(param.createIssue, false);
  EXPECT_EQ(param.testID, "TestID");

  EXPECT_TRUE(std::filesystem::exists(param.outputPath));
  EXPECT_TRUE(std::filesystem::exists(param.imagesPath));
  EXPECT_TRUE(std::filesystem::exists(param.logsPath));

  std::filesystem::remove_all(param.outputPath);
}

TEST_F(ConfigurationHandlerTest, InvalidDatatypeBool)
{
  config.configFilename = "../test/TEST3_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

TEST_F(ConfigurationHandlerTest, InvalidDatatypeInt)
{
  config.configFilename = "../test/TEST4_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

TEST_F(ConfigurationHandlerTest, InvalidDatatypeString)
{
  config.configFilename = "../test/TEST5_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

TEST_F(ConfigurationHandlerTest, InvalidLogLevel)
{
  config.configFilename = "../test/TEST6_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

TEST_F(ConfigurationHandlerTest, InvalidInterval)
{
  config.configFilename = "../test/TEST7_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

TEST_F(ConfigurationHandlerTest, InvalidOutputPathEmpty)
{
  config.configFilename = "../test/TEST8_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

TEST_F(ConfigurationHandlerTest, InvalidOutputPathBadCharacter)
{
  config.configFilename = "../test/TEST9_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

TEST_F(ConfigurationHandlerTest, InvalidTestID)
{
  config.configFilename = "../test/TEST10_configParameter.json";
  EXPECT_FALSE(config.loadConfiguration(param));
}

//=============================================================================
//================= ImageSavePreparerTest =====================================
//=============================================================================
class ImageSavePreparerTest : public ::testing::Test
{
protected:
  ImageSavePreparer save;
  ConfigurationParameter param;

  void SetUp() override
  {
    param.imagesPath = "../tmp";
    param.newFolderInterval = 10;
    std::filesystem::create_directory(param.imagesPath);
  }

  void TearDown() override
  {
    std::filesystem::remove_all(param.imagesPath);
  }
};

TEST_F(ImageSavePreparerTest, NewNumberedFolderAndImages)
{
  param.numberFolders = true;
  param.numberImages = true;
  int imageNumber = 10;
  EXPECT_EQ(save.prepareImageSave(param, imageNumber), "../tmp/1/image_10.png");
}

TEST_F(ImageSavePreparerTest, SameFolderSequentialImages)
{
  param.numberFolders = true;
  param.numberImages = true;
  int imageNumber = 11;
  EXPECT_EQ(save.prepareImageSave(param, imageNumber), "../tmp/2/image_11.png");
}

TEST_F(ImageSavePreparerTest, NoNumberedFoldersButNumberedImages)
{
  param.numberFolders = false;
  param.numberImages = true;
  int imageNumber = 1;
  std::string path = save.prepareImageSave(param, imageNumber);
  EXPECT_TRUE(path.find("image_1.png") != std::string::npos);
}

TEST_F(ImageSavePreparerTest, NoNumberedFoldersSequentialImages)
{
  param.numberFolders = false;
  param.numberImages = true;
  int imageNumber = 1;
  std::string path1 = save.prepareImageSave(param, imageNumber);
  imageNumber = 2;
  std::string path2 = save.prepareImageSave(param, imageNumber);
  EXPECT_TRUE(path1.find("image_1.png") != std::string::npos);
  EXPECT_TRUE(path2.find("image_2.png") != std::string::npos);
  EXPECT_EQ(path1.substr(0, path1.find("image_")), path2.substr(0, path2.find("image_")));
}

TEST_F(ImageSavePreparerTest, NumberedFoldersNotImages)
{
  param.numberFolders = true;
  param.numberImages = false;
  int imageNumber = 10;
  std::string path = save.prepareImageSave(param, imageNumber);
  EXPECT_TRUE(path.find("image_") != std::string::npos);
  EXPECT_TRUE(path.find(".png") != std::string::npos);
}

TEST_F(ImageSavePreparerTest, NoNumberedFoldersOrImages)
{
  param.numberFolders = false;
  param.numberImages = false;
  int imageNumber = 1;
  std::string path = save.prepareImageSave(param, imageNumber);
  EXPECT_TRUE(path.find("image_") != std::string::npos);
  EXPECT_TRUE(path.find(".png") != std::string::npos);
}
TEST_F(ImageSavePreparerTest, CreateNumberedFolder)
{
  param.numberFolders = true;
  param.numberImages = true;
  int imageNumber = 1;
  save.prepareImageSave(param, imageNumber);
  EXPECT_TRUE(std::filesystem::exists("../tmp/1"));
}

TEST_F(ImageSavePreparerTest, ReuseLastSubpath)
{
  param.numberFolders = false;
  param.numberImages = true;
  int imageNumber = 1;
  std::string path1 = save.prepareImageSave(param, imageNumber);
  imageNumber = 2;
  std::string path2 = save.prepareImageSave(param, imageNumber);
  EXPECT_EQ(path1.substr(0, path1.find("image_")), path2.substr(0, path2.find("image_")));
}

TEST_F(ImageSavePreparerTest, FolderWithTimestamp)
{
  param.numberFolders = false;
  param.numberImages = true;
  int imageNumber = 1;
  std::string path = save.prepareImageSave(param, imageNumber);
  std::string expected_folder = path.substr(0, path.find("image_"));
  EXPECT_TRUE(std::filesystem::exists(expected_folder));
}

TEST_F(ImageSavePreparerTest, FilenameWithTimestamp)
{
  param.numberFolders = true;
  param.numberImages = false;
  int imageNumber = 10;
  std::string path = save.prepareImageSave(param, imageNumber);
  EXPECT_TRUE(path.find("image_") != std::string::npos);
  EXPECT_TRUE(path.find(".png") != std::string::npos);
}

TEST_F(ImageSavePreparerTest, NumFoldersNumImages_NewFolder)
{
  param.numberFolders = true;
  param.numberImages = true;
  int imageNumber = 10;
  int partNumber = 0;
  EXPECT_EQ(save.prepareBufferSave(param, imageNumber, partNumber), "../tmp/10/image_10part.0.png");
}

TEST_F(ImageSavePreparerTest, NumFoldersNumImages_SameFolder)
{
  param.numberFolders = true;
  param.numberImages = true;
  int imageNumber = 10;
  int partNumber = 1;
  EXPECT_EQ(save.prepareBufferSave(param, imageNumber, partNumber), "../tmp/10/image_10part.1.png");
}

TEST_F(ImageSavePreparerTest, NoNumFoldersNumImages)
{
  param.numberFolders = false;
  param.numberImages = true;
  int imageNumber = 10;
  int partNumber = 0;
  std::string path = save.prepareBufferSave(param, imageNumber, partNumber);
  EXPECT_TRUE(path.find("image_10part.0.png") != std::string::npos);
}

TEST_F(ImageSavePreparerTest, NoNumFoldersNumImages_SameFolder)
{
  param.numberFolders = false;
  param.numberImages = true;
  int imageNumber = 10;
  int partNumber = 0;
  std::string path1 = save.prepareBufferSave(param, imageNumber, partNumber);
  imageNumber = 10;
  partNumber = 1;
  std::string path2 = save.prepareBufferSave(param, imageNumber, partNumber);
  EXPECT_TRUE(path1.find("image_10part.0.png") != std::string::npos);
  EXPECT_TRUE(path2.find("image_10part.1.png") != std::string::npos);
  EXPECT_EQ(path1.substr(0, path1.find("image_")), path2.substr(0, path2.find("image_")));
}

TEST_F(ImageSavePreparerTest, NumFoldersNoNumImages)
{
  param.numberFolders = true;
  param.numberImages = false;
  int imageNumber = 10;
  int partNumber = 0;
  std::string path = save.prepareBufferSave(param, imageNumber, partNumber);
  EXPECT_TRUE(path.find("image_10_") != std::string::npos);
  EXPECT_TRUE(path.find(".png") != std::string::npos);
}
TEST_F(ImageSavePreparerTest, NoNumFoldersNoNumImages)
{
  param.numberFolders = false;
  param.numberImages = false;
  int imageNumber = 10;
  int partNumber = 0;
  std::string path = save.prepareBufferSave(param, imageNumber, partNumber);
  EXPECT_TRUE(path.find("image_10_") != std::string::npos);
  EXPECT_TRUE(path.find(".png") != std::string::npos);
}

TEST_F(ImageSavePreparerTest, CreateNewFolder)
{
  param.numberFolders = true;
  param.numberImages = true;
  int imageNumber = 10;
  int partNumber = 0;
  save.prepareBufferSave(param, imageNumber, partNumber);
  EXPECT_TRUE(std::filesystem::exists("../tmp/10"));
}
TEST_F(ImageSavePreparerTest, ReuseSubpath)
{
  param.numberFolders = false;
  param.numberImages = true;
  int imageNumber = 10;
  int partNumber = 0;
  std::string path1 = save.prepareBufferSave(param, imageNumber, partNumber);
  imageNumber = 10;
  partNumber = 1;
  std::string path2 = save.prepareBufferSave(param, imageNumber, partNumber);
  EXPECT_EQ(path1.substr(0, path1.find("image_")), path2.substr(0, path2.find("image_")));
}

TEST_F(ImageSavePreparerTest, DifferentFolderInterval)
{
  param.numberFolders = true;
  param.numberImages = true;
  param.newFolderInterval = 5;
  int imageNumber = 5;
  int partNumber = 0;
  save.prepareBufferSave(param, imageNumber, partNumber);
  EXPECT_TRUE(std::filesystem::exists("../tmp/5"));
}

//=============================================================================
//================= LoggerTest ================================================
//=============================================================================
class LoggerTest : public ::testing::Test
{
protected:
  Logger log;
  ConfigurationParameter param;

  void SetUp() override
  {
    param.logsPath = "/tmp";
    param.maxLogSize = 1;   // 1 MB
    param.maxLogFiles = 3;
    param.logLevel = 2;     // warn level
  }

  void TearDown() override
  {
    spdlog::drop("logClient");
  }
};

TEST_F(LoggerTest, InitializeLogger)
{
  log.initializeLogger(param);
  auto logger = spdlog::get("logClient");

  ASSERT_NE(logger, nullptr);

  EXPECT_EQ(logger->level(), spdlog::level::warn);

  EXPECT_EQ(logger->flush_level(), spdlog::level::trace);

  std::filesystem::path logFilePath("/tmp/Logfile.txt");
  EXPECT_TRUE(std::filesystem::exists(logFilePath));
}

TEST_F(LoggerTest, InitializeLoggerWithDifferentLogLevels)
{
  param.logLevel = 0;
  log.initializeLogger(param);
  auto logger0 = spdlog::get("logClient");
  ASSERT_NE(logger0, nullptr);
  EXPECT_EQ(logger0->level(), spdlog::level::off);
  spdlog::drop("logClient");

  param.logLevel = 1;
  log.initializeLogger(param);
  auto logger1 = spdlog::get("logClient");
  ASSERT_NE(logger1, nullptr);
  EXPECT_EQ(logger1->level(), spdlog::level::err);
  spdlog::drop("logClient");

  param.logLevel = 2;
  log.initializeLogger(param);
  auto logger2 = spdlog::get("logClient");
  ASSERT_NE(logger2, nullptr);
  EXPECT_EQ(logger2->level(), spdlog::level::warn);
  spdlog::drop("logClient");

  param.logLevel = 3;
  log.initializeLogger(param);
  auto logger3 = spdlog::get("logClient");
  ASSERT_NE(logger3, nullptr);
  EXPECT_EQ(logger3->level(), spdlog::level::info);
  spdlog::drop("logClient");
}

//=============================================================================
//================= AzureIssueCreatorTest =====================================
//=============================================================================
class AzureIssueCreatorTest : public ::testing::Test
{
protected:
  AzureIssueCreator issueCreator;
  ConfigurationParameter param;
  Logger log;
  int frameCount = 10;

  void SetUp() override
  {
    param.createIssue = true;
    param.createLog = true;
    param.maxLogFiles = 3;
    param.maxLogSize = 5;
    param.logLevel = 3;
    param.testID = "12345";
    param.logsPath = "../tmp";
  }

  bool logContains(const std::string &substring)
  {
    std::ifstream logFile("../tmp/Logfile.txt");
    std::string line;
    while (getline(logFile, line))
    {
      if (line.find(substring) != std::string::npos)
      {
        return true;
      }
    }
    return false;
  }
};

TEST_F(AzureIssueCreatorTest, TestSendAzureIssueFailure)
{
  log.initializeLogger(param);
  param.createIssue = false;
  issueCreator.sendAzureIssue(param, frameCount);
  ASSERT_FALSE(logContains("AzureIssue created: Title: Test: 12345 failed!, Description: Test failed / Time:"));
  spdlog::drop("logClient");
  std::filesystem::remove_all("../tmp");
}

// The MockSystemCalls class simulates system functions like tcgetattr and fcntl for predictable unit testing without real system dependencies
class MockSystemCalls
{
public:
  MOCK_METHOD(int, tcgetattr, (int fd, struct termios *t), ());
  MOCK_METHOD(int, tcsetattr, (int fd, int actions, const struct termios *t), ());
  MOCK_METHOD(int, fcntlGet, (int fd), ());
  MOCK_METHOD(int, fcntlSet, (int fd, int flags), ());
  MOCK_METHOD(int, getchar, (), ());
};

MockSystemCalls *globalSystemMock;

// Defines external "C" functions that replace standard library functions, redirecting calls to mock implementations
// for controlled behavior verification in unit tests
extern "C"
{
  int tcgetattr(int fd, struct termios *t)
  {
    return globalSystemMock->tcgetattr(fd, t);
  }
  int tcsetattr(int fd, int actions, const struct termios *t)
  {
    return globalSystemMock->tcsetattr(fd, actions, t);
  }
  int fcntl(int fd, int cmd, ...)
  {
    va_list args;
    va_start(args, cmd);
    int result = 0;
    if (cmd == F_GETFL)
    {
      result = globalSystemMock->fcntlGet(fd);
    }
    else if (cmd == F_SETFL)
    {
      int flags = va_arg(args, int);
      result = globalSystemMock->fcntlSet(fd, flags);
    }
    va_end(args);
    return result;
  }
  int getchar()
  {
    return globalSystemMock->getchar();
  }
}

TEST_F(AzureIssueCreatorTest, ReturnsTrueWhenKeyIsHit)
{
  MockSystemCalls systemMock;
  globalSystemMock = &systemMock;

  EXPECT_CALL(systemMock, tcgetattr(0, testing::_)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, tcsetattr(0, TCSANOW, testing::_)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, fcntlGet(0)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, fcntlSet(0, O_NONBLOCK)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, fcntlSet(0, 0)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, getchar()).WillRepeatedly(testing::Return('A'));

  AzureIssueCreator creator;

  EXPECT_TRUE(creator.checkKeyboardHit());
  globalSystemMock = nullptr;
}

TEST_F(AzureIssueCreatorTest, ReturnsFalseWhenNoKeyIsHit)
{
  MockSystemCalls systemMock;
  globalSystemMock = &systemMock;

  EXPECT_CALL(systemMock, tcgetattr(0, testing::_)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, tcsetattr(0, TCSANOW, testing::_)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, fcntlGet(0)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, fcntlSet(0, O_NONBLOCK)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, fcntlSet(0, 0)).WillRepeatedly(testing::Return(0));
  EXPECT_CALL(systemMock, getchar()).WillRepeatedly(testing::Return(EOF));

  AzureIssueCreator creator;

  EXPECT_FALSE(creator.checkKeyboardHit());
  globalSystemMock = nullptr;
}

//=============================================================================
//================= main ======================================================
//=============================================================================
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}