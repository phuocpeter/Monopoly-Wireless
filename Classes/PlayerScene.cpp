//
//  PlayerScene.cpp
//  MGM
//
//  Created by Tran Thai Phuoc on 2016-06-13.
//
//

#include "PlayerScene.h"
#include "BankScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include "extensions/cocos-ext.h"
#include "network/HttpClient.h"
#include "editor-support/spine/Json.h"
#include <vector>

USING_NS_CC;
USING_NS_CC_EXT;

char* jsonString;
std::string serverIP;
std::string message;

Scene* PlayerScene::createScene(char* js, std::string serverAddress, std::string info)
{
  // Initialize parameters from json
  jsonString = js;
  serverIP = serverAddress;
  message = info;
  
  // 'scene' is an autorelease object
  auto scene = Scene::create();
  
  // 'layer' is an autorelease object
  auto layer = PlayerScene::create();
  
  // add layer as a child to scene
  scene->addChild(layer);
  
  // return the scene
  return scene;
}

// on "init" you need to initialize your instance
bool PlayerScene::init()
{
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
  {
    return false;
  }
  
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // Initialize data from JSON
  Json* json = Json_create(jsonString);
  _sessionID = Json_getString(json, "sessionID", "NULL");
  _numOfPlayers = Json_getInt(json, "numOfPlayers", -1);
  _playerName = Json_getString(json, "playerName", "NULL");
  int money = Json_getInt(json, "money", -1);
  int playerNo = Json_getInt(json, "playerNo", -1);
  
  // Initialize players for player slider
  switch (playerNo) {
    case 1:
      _player1 = Json_getString(json, "2", "NULL");
      _player2 = Json_getString(json, "3", "NULL");
      _player3 = Json_getString(json, "4", "NULL");
      break;
    case 2:
      _player1 = Json_getString(json, "1", "NULL");
      _player2 = Json_getString(json, "3", "NULL");
      _player3 = Json_getString(json, "4", "NULL");
      break;
    case 3:
      _player1 = Json_getString(json, "1", "NULL");
      _player2 = Json_getString(json, "2", "NULL");
      _player3 = Json_getString(json, "4", "NULL");
      break;
    case 4:
      _player1 = Json_getString(json, "1", "NULL");
      _player2 = Json_getString(json, "2", "NULL");
      _player3 = Json_getString(json, "3", "NULL");
      break;
    default:
      _player1 = "NULL";
      _player2 = "NULL";
      _player3 = "NULL";
      break;
  }
  
  // Session Label
  std::string sessionString = "Mã Game: " + (std::string)_sessionID;
  auto sessionLabel = Label::createWithTTF(sessionString, "fonts/arial.ttf", 20.0f);
  //sessionLabel->setAnchorPoint(Vec2(0.5, 1));
  sessionLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height - sessionLabel->getContentSize().height));
  this->addChild(sessionLabel);
  
  // Player Label
  std::string playerString = "Tên: " + (std::string)_playerName;
  auto playerLabel = Label::createWithTTF(playerString, "fonts/arial.ttf", 20.0f);
  playerLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                sessionLabel->getPositionY() - 40));
  this->addChild(playerLabel);
  
  // Money Label
  std::string moneyString = "Tiền: " + std::to_string(money);
  auto moneyLabel = Label::createWithTTF(moneyString, "fonts/arial.ttf", 20.0f);
  moneyLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                               playerLabel->getPositionY() - 40));
  this->addChild(moneyLabel);
  
  // Transfer Label
  auto transferLabel = Label::createWithTTF("GIAO DỊCH", "fonts/arial.ttf", 20.0f);
  transferLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                  moneyLabel->getPositionY() - 40));
  this->addChild(transferLabel);
  
  // Player Slider
  auto slider = ui::Slider::create();
  slider->loadBarTexture("Slider_Back.png");
  slider->loadSlidBallTextureNormal("SliderNode_Normal.png");
  slider->loadSlidBallTexturePressed("SliderNode_Press.png");
  slider->loadSlidBallTextureDisabled("SliderNode_Disable.png");
  slider->setPosition(Vec2(origin.x + visibleSize.width / 2,
                           transferLabel->getPositionY() - 40));
  // Disable the slider if there are less than 2 player
  slider->setPercent(12.5);
  slider->setEnabled(_numOfPlayers > 1);
  slider->addEventListener(CC_CALLBACK_2(PlayerScene::sliderEvent, this));
  this->addChild(slider);
  
  // Recipent Label
  _recipentLabel = Label::createWithTTF("Ngân Hàng", "fonts/arial.ttf", 20.0f);
  _recipentLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, slider->getPositionY() - 40));
  this->addChild(_recipentLabel);
  _recipentName = "Bank";
  
  // Amount Textfield
  auto amountField = ui::TextField::create("Số Lượng Để Chuyển", "fonts/arial.ttf", 20.0f);
  amountField->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                _recipentLabel->getPositionY() - 40));
  amountField->addEventListener(CC_CALLBACK_2(PlayerScene::amountTextFieldEvent, this));
  this->addChild(amountField);
  
  // Send button
  _sendBtn = ui::Button::create("touchid-64.png");
  _sendBtn->setPosition(Vec2(origin.x + visibleSize.width / 2 - 48,
                             amountField->getPositionY() - 50));
  _sendBtn->addTouchEventListener(CC_CALLBACK_2(PlayerScene::sendButtonTapped, this));
  this->addChild(_sendBtn);
  
  // Refresh button
  _refreshBtn = ui::Button::create("refresh-64.png");
  _refreshBtn->setPosition(Vec2(origin.x + visibleSize.width / 2 + 48,
                                amountField->getPositionY() - 50));
  _refreshBtn->addTouchEventListener(CC_CALLBACK_2(PlayerScene::refreshButtonTapped, this));
  this->addChild(_refreshBtn);
  
  // Loading Bar placeholder
  _loadingPlaceholder = Sprite::create("Slider_Back.png");
  _loadingPlaceholder->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                        _refreshBtn->getPositionY() - 50));
  _loadingPlaceholder->setVisible(false);
  this->addChild(_loadingPlaceholder);
  
  // Loading Bar
  _loadingBar = ui::LoadingBar::create("Slider_PressBar.png");
  _loadingBar->setPercent(0);
  _loadingBar->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                _refreshBtn->getPositionY() - 50));
  this->addChild(_loadingBar);
  
  // Error Label
  _errorLabel = Label::createWithTTF(message, "fonts/arial.ttf", 20.0f);
  _errorLabel->setTextColor(Color4B::GREEN);
  _errorLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                _sendBtn->getPositionY() - 50));
  this->addChild(_errorLabel);
  
  // Create HTTP Request
  _request = new network::HttpRequest();
  _request->setRequestType(network::HttpRequest::Type::POST);
  _request->setTag("POST playerTransfer");
  _request->setUrl(("http://" + serverIP + "/monopoly/").c_str());
  _request->setResponseCallback(CC_CALLBACK_2(PlayerScene::onHttpRequestCompleted, this));
  
  // Display Bank tab if the player is host
  if (playerNo == 1) {
    // Create menu tab
    auto playerMenuLabel = Label::createWithTTF("Người C  1hơi", "fonts/arial.ttf", 20.0f);
    auto playerTab = MenuItemLabel::create(playerMenuLabel);
    playerTab->setEnabled(true);
    
    auto bankLabel = Label::createWithTTF("Ngân Hàng", "fonts/arial.ttf", 20.0f);
    auto bank = MenuItemLabel::create(bankLabel);
    bank->setEnabled(true);
    bank->setCallback(CC_CALLBACK_1(PlayerScene::bankSceneChange, this));
    
    auto menu = Menu::create(playerTab, bank, NULL);
    menu->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
    menu->alignItemsHorizontallyWithPadding(visibleSize.width / 4);
    this->addChild(menu);
  }
  
  return true;
}

// MARK: - TextField Handler

void PlayerScene::amountTextFieldEvent(cocos2d::Ref *sender, ui::TextField::EventType type) {
  ui::TextField* field = dynamic_cast<ui::TextField*>(sender);
  switch (type) {
    case ui::TextField::EventType::INSERT_TEXT:
    case ui::TextField::EventType::DELETE_BACKWARD:
      _amount = atoi(field->getString().c_str());
      break;
    case ui::TextField::EventType::ATTACH_WITH_IME:
      _sendBtn->setEnabled(false);
      break;
    case ui::TextField::EventType::DETACH_WITH_IME:
      _sendBtn->setEnabled(true);
      break;
    default:
      break;
  }
}

// MARK: - Button Handler

void PlayerScene::sendButtonTapped(cocos2d::Ref *sender, ui::Widget::TouchEventType type) {
  _loadingPlaceholder->setVisible(true);
  _errorLabel->setString("");
  switch (type) {
    case ui::Widget::TouchEventType::ENDED:
      if (_amount > 0) {
        _loadingBar->setPercent(5);
        // Execute when amount is entered
        _plainSendData = "fromBank=false&recipent=" + _recipentName + "&amount=" + std::to_string(_amount) + "&sessionID=" + _sessionID + "&sender=" + _playerName;
        _dataToSend = _plainSendData.c_str();
        _loadingBar->setPercent(10);
        _request->setRequestData(_dataToSend, strlen(_dataToSend));
        network::HttpClient::getInstance()->send(_request);
        _loadingBar->setPercent(20);
      } else {
        // Display error
        _loadingPlaceholder->setVisible(false);
        _loadingBar->setPercent(0);
        _errorLabel->setTextColor(Color4B::RED);
        _errorLabel->setString("Vui lòng nhập đầy đủ thông tin");
        _errorLabel->setVisible(true);
      }
      break;
    default:
      break;
  }
}

void PlayerScene::refreshButtonTapped(cocos2d::Ref *sender, ui::Widget::TouchEventType type) {
  _loadingPlaceholder->setVisible(true);
  _errorLabel->setString("");
  switch (type) {
    case ui::Widget::TouchEventType::ENDED:
      _loadingBar->setPercent(5);
      // Concatenate the plain data string
      _plainSendData = "playerName=" + _playerName + "&sessionID=" + _sessionID;
      // Turn plain data string to const char
      _dataToSend = _plainSendData.c_str();
      _loadingBar->setPercent(10);
      // Set _request's parameters
      _request->setUrl(("http://" + serverIP + "/monopoly/").c_str());
      _request->setRequestData(_dataToSend, strlen(_dataToSend));
      network::HttpClient::getInstance()->send(_request);
      _loadingBar->setPercent(20);
      break;
    default:
      break;
  }
}

// MARK: - Network Handler

void PlayerScene::onHttpRequestCompleted(network::HttpClient *sender, network::HttpResponse *response) {
  printf("\n");
  // Print connection status
  if (response->getResponseCode() == 200) {
    printf("Connected successfully\n");
    printf("Transfering money from player\n");
    
    _loadingBar->setPercent(40);
    std::vector<char> *buffer = response->getResponseData();
    
    // Convert the response data to plain string
    char *concatenated = (char*)malloc(buffer->size());
    std::string string2(buffer->begin(), buffer->end());
    strcpy(concatenated, string2.c_str());
    printf("%s\n", concatenated);
    _loadingBar->setPercent(50);
    
    Json* json = Json_create(concatenated);
    auto errorCode = Json_getInt(json, "errorCode", -99);
    _loadingBar->setPercent(75);
    
    if (errorCode == -1) {
      // No error, switch to next scene
      _loadingBar->setPercent(100);
      auto scene = PlayerScene::createScene(concatenated, serverIP, "Xong!");
      auto director = Director::getInstance();
      director->replaceScene(scene);
    } else {
      // Error from server
      auto message = Json_getString(json, "message", "NULL");
      _loadingPlaceholder->setVisible(false);
      _loadingBar->setPercent(0);
      _errorLabel->setTextColor(Color4B::RED);
      _errorLabel->setString(message);
      _errorLabel->setVisible(true);
      printf("%s\n", message);
    }
  } else {
    // Failed to connect to server
    _loadingPlaceholder->setVisible(false);
    _loadingBar->setPercent(0);
    _errorLabel->setTextColor(Color4B::RED);
    _errorLabel->setString("Không thể kết nỗi tới Server\nLỗi: " + std::to_string(response->getResponseCode()));
    _errorLabel->setVisible(true);
    printf("Error: %li", response->getResponseCode());
  }
}

// MARK: - Scene change

void PlayerScene::bankSceneChange(cocos2d::Ref *sender) {
  auto scene = BankScene::createScene(jsonString, serverIP, "");
  auto director = Director::getInstance();
  director->replaceScene(scene);
}

// MARK: - Slider Event Handler

void PlayerScene::sliderEvent(cocos2d::Ref *sender, ui::Slider::EventType type) {
  // When slider changes value
  // 12.5, 37.5, 62.5 and 87.5 are 4 values on the slider
  // represent 4 players
  ui::Slider *slider = dynamic_cast<ui::Slider*>(sender);
  auto percent = slider->getPercent();
  if (percent <= 12.5f) {
    slider->setPercent(12.5f);
    _recipentLabel->setString("Ngân Hàng");
    _recipentName = "Bank";
  } else if (percent <= 37.5) {
    slider->setPercent(37.5f);
    if (_numOfPlayers >= 2) {
      _recipentLabel->setString(_player1);
      _recipentName = _player1;
    } else {
      _recipentLabel->setString("No Player");
    }
  } else if (percent <= 62.5) {
    slider->setPercent(62.5f);
    if (_numOfPlayers >= 3) {
      _recipentLabel->setString(_player2);
      _recipentName = _player2;
    } else {
      _recipentLabel->setString("No Player");
    }
  } else {
    slider->setPercent(87.5f);
    if (_numOfPlayers == 4) {
      _recipentLabel->setString(_player3);
      _recipentName = _player3;
    } else {
      _recipentLabel->setString("No Player");
    }
  }
}