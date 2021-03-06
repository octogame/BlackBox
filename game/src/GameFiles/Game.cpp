﻿#include <Game.hpp>
#include <GameObject.hpp>

#include <BlackBox/Renderer/Camera.hpp>
#include <BlackBox/Renderer/IFont.hpp>
#include <BlackBox/Renderer/IRender.hpp>
#include <BlackBox/Renderer/Material.hpp>
#include <BlackBox/Renderer/TechniqueManager.hpp>
#include <BlackBox/Renderer/Texture.hpp>
#include <BlackBox/Resources/SceneManager.hpp>
#include <BlackBox/Scene/IScene.hpp>

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {
#if 0
  sf::RenderWindow& getWindow()
  {
    return *static_cast<sf::RenderWindow*>(GetISystem()->GetIWindow()->getHandle());
  }
#endif
}

struct TextRenderInfo
{
  IFont* font;
  std::vector<std::string> text;
  Vec4 color;
  SDrawTextInfo dti;
  TextRenderInfo() : font(nullptr), color(Vec4(1.0)) {}
  TextRenderInfo(IFont* f, Vec4 c)
    :
    font(f), color(c)
  {
  }
  void AddLine(std::string line)
  {
    text.push_back(line + '\n');
  }
  SDrawTextInfo& getDTI()
  {
    dti.color[0] = color[0];
    dti.color[1] = color[1];
    dti.color[2] = color[2];
    dti.color[3] = color[3];
    dti.font = font;
    return dti;
  }
};

IWorld* CGame::getWorld() const
{
  return m_World;
}

void CGame::PreRender()
{
  //glCheck(glViewport(0, 0, 1366, 768));
}

CGame::CGame(std::string title) :
  //camControl(nullptr),
  g_scene(nullptr),
#ifdef GUI
  gui(nullptr),
#endif // GUI
  listener(nullptr),
  m_Console(nullptr),
  m_Font(nullptr),
  m_pLog(nullptr),
  //m_ScreenShader(nullptr),
  m_inputHandler(nullptr),
  m_player(nullptr),
  m_pSystem(nullptr),
  m_scene(nullptr),
  m_sceneManager(nullptr),
  //shaderManager(nullptr),
  m_World(nullptr),
  m_Title(title),
  m_pScriptObjectGame(nullptr),
  m_pRender(nullptr),
  m_pInput(nullptr),
  m_pScriptSystem(nullptr),
  r_displayinfo(nullptr),
  r_cap_profile(nullptr),
  r_profile(nullptr),
  m_pCVarCheatMode(nullptr),

  m_currentLevelFolder("tmp")
{
#pragma warning(push)
#pragma warning(disable : 4244)
  srand(time(nullptr));
#pragma warning(push)
  m_deltaTime = 0.0f;
  m_lastTime = 0.0f;
#ifdef ENABLE_MUSIC_LIST
  m_PlayList.setRootPath("res/music/");
  m_PlayList.addTrack("background.ogg");
  m_PlayList.addTrack("japan.ogg");
#endif
}

bool CGame::Init(ISystem* pEngine) {
  m_pSystem /*= gISystem */ = pEngine;
  m_pRender = m_pSystem->GetIRender();
  m_pInput = m_pSystem->GetIInput();
  m_pScriptSystem = m_pSystem->GetIScriptSystem();
  m_pLog = m_pSystem->GetILog();
  m_Console = m_pSystem->GetIConsole();
  m_pInput->AddEventListener(this);
  m_pNetwork = m_pSystem->GetINetwork();
  m_World = m_pSystem->GetIWorld();

#if 0
  if (!m_pNetwork->Init())
    return false;

  m_pClient = m_pNetwork->CreateClient();
  if (!m_pClient || !m_pClient->Init())
    return false;

  m_pServer = m_pNetwork->CreateServer();
  if (!m_pServer)
    return false;
#endif

  initCommands();
  initVariables();
  InitScripts();
  auto init_cfg = m_Console->GetCVar("game_config");
  if (init_cfg == nullptr)
  {
    //TODO: log: game config not specified
    return false;
  }
  m_Console->ExecuteFile((std::string("res/scripts/") + init_cfg->GetString()).c_str());
  auto is_complete = m_Console->GetCVar("g_init_complete");
  if (is_complete == nullptr)
  {
    //TODO: log: error load Init.cfg
    return false;
  }

  InitInputMap();
  InitConsoleCommands();

  DevModeInit();

#if 0
  if (!loadScene()) {
    m_pLog->Log("[FAILED] Failed init objects\n");
    return false;
  }
#endif
  // Set scene before camera, camera setted to active scene in world
  if (m_scene)
    m_World->SetScene(m_scene);
#ifdef GUI
  gui = new GameGUI();
  gui->game = this;
#endif // GUI
  initPlayer();
  m_pInput->ShowCursor(false);
  m_pInput->GrabInput(true);

  if (m_scene != nullptr)
  {
    initTechniques();
  }
  else
    TechniqueManager::init();

  m_Font = m_pSystem->GetIFont();
  m_Font->Init("arial.ttf", 16, 18);

  ITexture* consoleBackGround = new Texture();
#if 0
  consoleBackGround->load("console/fc.jpg");
#endif
  m_Console->SetImage(consoleBackGround);

  // other
  //TODO: FIX IT
#if 0
  mousePrev = sf::Mouse::getPosition();
  mouseDelta = sf::Vector2i(0, 0);
#endif
#if CURSOR_FIXED
  cursor.loadFromSystem(sf::Cursor::Arrow);
  m_pSystem->GetIWindow()->setCursor(reinterpret_cast<Cursor*>(&cursor));
#endif

  m_Console->ExecuteFile("res/scripts/postinit.cfg");
  if (m_Console->GetCVar("nsightDebug"))
  {
    m_Console->ExecuteString("r_displayinfo 0");
  }

  return true;
}

bool CGame::Update() {
  bool bRenderFrame = true;
  m_pSystem->Update(0, IsInPause());
  {
    m_SceneRendered = false;
    // TODO: FIX IT
    m_deltaTime = m_pSystem->GetDeltaTime();
    m_time += m_deltaTime;
    fps = 1.0f / m_deltaTime;
    ExecScripts();
    if (!IsInPause())
      m_World->Update(m_pSystem->GetDeltaTime());

    SetRenderState();
    if (bRenderFrame)
    {
      m_pSystem->RenderBegin();
      m_pSystem->Render();
      //PROFILER_PUSH_CPU_MARKER("DrawHud", Utils::COLOR_CYAN);
      DrawHud(fps);
      //PROFILER_POP_CPU_MARKER();
    }
  }
  //////////////////////////////////////////////////////////////////////////
  // Special update function for developers mode.
  //////////////////////////////////////////////////////////////////////////
  if (IsDevModeEnable() && false)
    DevModeUpdate();
  //////////////////////////////////////////////////////////////////////////
  while (!m_qMessages.empty())
  {
    string smsg = m_qMessages.front();
    m_qMessages.pop();
    ProcessPMessages(smsg.c_str());
  }
  //////////////////////////////////////////////////////////////////////////

  if (bRenderFrame)
    m_pSystem->RenderEnd();

  return m_bUpdateRet;
}

void CGame::ExecScripts()
{
}

void CGame::DrawHud(float fps)
{
  m_pRender->SetViewport(0, 0, m_pRender->GetWidth(), m_pRender->GetHeight());
  if (r_displayinfo->GetIVal() != 0)
  {
    DisplayInfo(fps);
  }
  if (m_Console->IsOpened())
  {
    m_Console->Draw();
  }
}

void CGame::DisplayInfo(float fps)
{
  size_t num_objects;
  if (m_World->GetActiveScene())
  {
    num_objects = m_World->GetActiveScene()->numObjects();
  }
  auto line = m_pRender->GetHeight();
  auto step = 18;

  std::string mode = m_Mode == MENU ? "MENU"
    : m_Mode == FPS ? "FPS"
    : m_Mode == FLY ? "FLY"
    : "EDIT";

  // Info
  TextRenderInfo info(m_Font, Vec4(0.5, 1.0f, 0.6f, 1.0));
  SDrawTextInfo dti = info.getDTI();

  //
  auto render = m_pSystem->GetIRender();

  //===========

  m_Font->SetXPos(0);
  m_Font->SetYPos(18);
  auto& text = info.text;
  auto& color = info.color;
  //auto camera = m_World->getActiveScene()->getCurrentCamera();

  //auto objPos = m_World->getActiveScene()->selectedObject()->second->m_transform.position;
  info.AddLine("FPS: " + std::to_string(fps));
  //info.AddLine("NUM OBJECTS: "			+ std::to_string(num_objects));
  info.AddLine("Current mode: " + mode);
  info.AddLine("Width = " + std::to_string(m_pRender->GetWidth()) + "Height = " + std::to_string(m_pRender->GetHeight()));
  //info.AddLine("Active scene: "			+ m_World->getActiveScene()->name);
  //info.AddLine("Selected Object: "	+ m_World->getActiveScene()->selectedObject()->first);
  //info.AddLine("  visible: "				+ std::to_string(m_World->getActiveScene()->selectedObject()->second->visible()));
  /*
  info.AddLine("  Pos: "						+
    std::to_string(objPos.x) + ", " +
    std::to_string(objPos.y) + ", " +
    std::to_string(objPos.z) + "; ");
  */
  //info.AddLine("Camera speed: " + std::to_string(camera->MovementSpeed->GetFVal()));
  //auto camPos = camera->getPosition();
  //auto camRot = camera->getRotation();
  /*
  auto pos = "Pos: " +
    std::to_string(camPos.x) + ", " +
    std::to_string(camPos.y) + ", " +
    std::to_string(camPos.z) + "; " +
    "Yaw: " +
    std::to_string(camRot.y) + "; " +
    "Pitch: " +
    std::to_string(camRot.x) + "; "
    ;
  */

  for (auto& text : info.text)
  {
    render->PrintLine(text.c_str(), dti);
  }

  render->PrintLine("To hide depth buffer press <;>\n", dti);
  render->PrintLine((std::string("Camera width = ") + std::to_string(GET_CVAR("r_cam_w")->GetIVal()) + "\n").c_str(), dti);
  render->PrintLine((std::string("Camera height = ") + std::to_string(GET_CVAR("r_cam_h")->GetIVal()) + "\n").c_str(), dti);

  info.color = Vec4(1.0f, 0.f, 0.f, 1.0f);
  //render->PrintLine(pos.c_str(), info.getDTI());
  if (canDragViewPortWidth)
    render->PrintLine("CanDrag\n", info.getDTI());
  if (mousePressed)
    render->PrintLine("Mouse pressed\n", info.getDTI());

  if (m_Mode == MENU)
  {
    //TODO: FIX THIS
#if 0
    auto c = sf::Mouse::getPosition(::getWindow());
    render->PrintLine(("Cursor: " + std::to_string(c.x) + std::string(", ") + std::to_string(m_pRender->GetHeight() - c.y)).c_str(), info.getDTI());
#endif
  }
}

bool CGame::Run(bool& bRelaunch) {
  m_pLog->Log("[OK] Game started\n");
  //TODO: FIX THIS
  //m_time = deltaClock.restart().asSeconds();
#ifdef ENABLE_MUSIC_LIST
  m_PlayList.setVolume(10.f);
  m_PlayList.play();
  m_isMusicPlaying = true;
#endif
  m_bRelaunch = false;
  {
    auto p = GET_CVAR("single_pass");
    if (p->GetIVal())
    {
#if 0
      SmartScriptObject test(m_pScriptSystem);
      if (!m_pScriptSystem->GetGlobalValue("Test", *test))
      {
        delete test;
        m_pSystem->Log("\002 ERROR: can't find Test table ");
        return false;
      }

      m_pScriptSystem->BeginCall("Test", "OnInit");
      m_pScriptSystem->PushFuncParam(*test);
      m_pScriptSystem->EndCall();

      Update();
      std::time_t t = std::time(nullptr);
      std::stringstream ss;
      ss << "screen_shots/tests/" << std::put_time(std::localtime(&t), "%H-%M-%S") << ".png";
      m_pLog->Log("Screenshot name: %s", ss.str().c_str());
      m_pRender->ScreenShot(ss.str().c_str());
#endif // 0

      return true;
    }
  }
  while (1)
  {
    if (!Update())
      break;
  }

  bRelaunch = m_bRelaunch;
  return true;
}

bool CGame::loadScene(std::string name) {
  IScene* scene;
  std::string& path = name;
  SceneManager::instance()->removeScene(path);
  if ((scene = SceneManager::instance()->getScene(path, this)) != nullptr)
  {
    m_World->SetScene(scene);
    auto tech = TechniqueManager::get("hdr");
    if (tech != nullptr)
    {
      tech->Init(m_World->GetActiveScene(), nullptr);
      scene->setTechnique(tech);
    }

    //scene->setCamera("main", new CCamera());
    CPlayer* player = static_cast<CPlayer*>(scene->getObject("MyPlayer"));
    if (player != nullptr)
    {
      player->attachCamera(scene->getCurrentCamera());
      player->setGame(this);
      this->setPlayer(player);
    }
    return true;
  }

  return false;
}

void CGame::unloadScene(std::string name)
{
  SceneManager::instance()->removeScene(name);
}

void CGame::saveScene(std::string name, std::string as)
{
  std::string& path = name;
  if (SceneManager::instance()->exist(path))
  {
    auto scene = SceneManager::instance()->getScene(path, this);
    scene->save(as.c_str());
  }
}

void CGame::SetRenderState()
{
  m_pRender->SetState(IRenderer::State::DEPTH_TEST, true);
  m_pRender->SetState(IRenderer::State::BLEND, false);
  if (culling)
  {
    m_pRender->SetState(IRenderer::State::CULL_FACE, true);
    m_pRender->SetCullMode(IRenderer::CullMode::BACK);
  }
  else
  {
    m_pRender->SetState(IRenderer::State::CULL_FACE, false);
  }
}

void CGame::setPlayer(CPlayer* player)
{
  m_player = player;
}

void CGame::setCamera(CCamera* camera)
{
  //m_active_camera = camera;
  //m_World->setCamera(camera);
}

IGame * CreateIGame(const char* title) {
  CGame* game = new CGame(title);
  return (game);
}

bool CGame::OnInputEvent(const SInputEvent& event)
{
  {
    bool retflag;
    bool retval = ShouldHandleEvent(event, retflag);
    if (retflag) return retval;
  }
  bool result = false;
  if (!IsInPause())
    OnInputEventProxy(event);
  PersistentHandler(event);
  return result;
}

void CGame::PersistentHandler(const SInputEvent& event)
{
  auto useBoxFilter = m_Console->GetCVar("bf");
  auto lpx = m_Console->GetCVar("lpx");
  auto lpy = m_Console->GetCVar("lpy");
  auto lpz = m_Console->GetCVar("lpz");
  ////////////////////////
  bool keyPressed = event.deviceType == eIDT_Keyboard && event.state == event.state == eIS_Pressed;
  bool control = event.modifiers & eMM_Ctrl;
  bool shift = event.modifiers & eMM_Shift;
  bool alt = event.modifiers & eMM_Alt;
  ////////////////////////
  if (keyPressed)
  {
    if (event.keyId == eKI_Semicolon)
      openShadowMap = !openShadowMap;
    if (event.keyId >= eKI_F1 && event.keyId <= eKI_F12)
    {
      if (control)
      {
        DevMode_SavePlayerPos(event.keyId - eKI_F1, "BloomTest");
      }
      else if (shift)
      {
        DevMode_LoadPlayerPos(event.keyId - eKI_F1, "BloomTest");
      }
    }
    switch (event.keyId)
    {
    case eKI_Up:
    {
      lpy->Set(lpy->GetFVal() + 0.5f);
      break;
    }
    case eKI_Down:
    {
      lpy->Set(lpy->GetFVal() - 0.5f);
      break;
    }
    case eKI_Left:
    {
      lpx->Set(lpx->GetFVal() - 0.5f);
      break;
    }
    case eKI_Right:
    {
      lpx->Set(lpx->GetFVal() + 0.5f);
      break;
    }
    case eKI_Insert:
    {
      bool ubf = useBoxFilter->GetIVal();
      useBoxFilter->Set(!ubf);
      break;
    }
    case eKI_Pause:
    {
      m_bInPause = !m_bInPause;
      break;
    }
    default:
      break;
    }
  }
}

void CGame::gotoGame()
{
  if (m_player != nullptr)
  {
    m_World->GetActiveScene()->getCurrentCamera()->mode = CCamera::Mode::FPS;
    m_Mode = FPS;
    m_pInput->ShowCursor(false);
    m_pInput->GrabInput(true);
    m_Console->ShowConsole(false);
  }
}

void CGame::gotoFly()
{
  m_Mode = FLY;
}

void CGame::gotoEdit()
{
  m_Mode = EDIT;
}

void CGame::showMenu()
{
}

bool CGame::initPlayer()
{
  if (!m_scene)
    return false;
  if (m_player != nullptr)
  {
    delete m_player;
  }
  if ((m_player = reinterpret_cast<CPlayer*>(m_scene->getObject("MyPlayer"))) != nullptr)
  {
    m_player->attachCamera(m_World->GetActiveScene()->getCurrentCamera());
    m_player->setGame(this);
    return true;
  }
  return false;
}

void CGame::initTechniques()
{
#if 0
  TechniqueManager::init();

  auto tech = new HdrTechnique();
  tech->Init(m_World->getActiveScene(), nullptr);
  m_World->getActiveScene()->setTechnique(tech);

  postProcessors.push_back(nullptr);
  postProcessors.push_back(new PostProcessor("negative"));
  postProcessors.push_back(new PostProcessor("grayscale"));
  postProcessors.push_back(new PostProcessor("kernel.outline"));
  postProcessors.push_back(new PostProcessor("kernel.blur"));
  m_World->getActiveScene()->setPostProcessor(postProcessors[0]);
#endif
}

bool CGame::FpsInputEvent(const SInputEvent& event)
{
  /*
  if (m_Console->IsOpened())
    return false;
  */

  ////////////////////////
  bool keyPressed = event.deviceType == eIDT_Keyboard && event.state == eIS_Pressed;
  bool control = event.modifiers & eMM_Ctrl;
  bool shift = event.modifiers & eMM_Shift;
  bool alt = event.modifiers & eMM_Alt;
  ////////////////////////
  auto camera = m_World->GetActiveScene()->getCurrentCamera();
  if (keyPressed)
  {
    switch (event.keyId)
    {
      /*
      case eKI_P:
        isWireFrame = !isWireFrame;
        return true;
      */
    case eKI_Backspace:
      return true;
    case eKI_M:
      camera->mode = CCamera::Mode::FLY;
      m_Mode = Mode::FLY;
      return true;
    case eKI_Escape:
      gotoMenu();
      return true;
    case eKI_P:
      camera->MovementSpeed->Set(camera->MovementSpeed->GetFVal() + 5.0f);
      return true;
    case eKI_E:
      //m_inputHandler->mouseLock(false);
      m_Mode = EDIT;
      return true;
    /*case eKI_M:
      camera->MovementSpeed->Set(camera->MovementSpeed->GetFVal() + 5.0f);
      return true;*/
    case eKI_B:
      culling = !culling;
      return true;
    case eKI_F1:
      m_World->GetActiveScene()->selectedObject()->second->m_Material->nextDiffuse();
      return true;
    case eKI_F9:
      if (shift)
        ++currPP;
      else
        --currPP;
      //m_World->GetActiveScene()->setPostProcessor(postProcessors[currPP % postProcessors.size()]);
      return true;
    case eKI_Enter:
      if (alt == true)
        gotoFullscreen();
      return true;
    }
  }
  return m_player->OnInputEvent(event);
  //return false;
}

bool CGame::FlyInputEvent(const SInputEvent& event)
{
  ////////////////////////
  bool keyPressed = event.deviceType == eIDT_Keyboard && event.state == eIS_Pressed;
  bool control = event.modifiers & eMM_Ctrl;
  bool shift = event.modifiers & eMM_Shift;
  bool alt = event.modifiers & eMM_Alt;
  ////////////////////////
  if (keyPressed)
  {
    switch (event.keyId)
    {
    case eKI_P:
      isWireFrame = !isWireFrame;
      return true;
    case eKI_Backspace:
      return true;
    case eKI_Space:
      gotoGame();
      return true;
    case eKI_Escape:
      //TODO: FIX IT
      //m_inputHandler->mouseLock(false);
      m_Mode = Mode::MENU;
      return true;
    }
  }
  return m_player->OnInputEvent(event);
}

bool CGame::MenuInputEvent(const SInputEvent& event)
{
  ////////////////////////
  bool keyPressed = event.deviceType == eIDT_Keyboard && event.state == eIS_Pressed;
  bool control = event.modifiers & eMM_Ctrl;
  bool shift = event.modifiers & eMM_Shift;
  bool alt = event.modifiers & eMM_Alt;
  ////////////////////////
  if (keyPressed)
  {
    switch (event.keyId)
    {
    case eKI_Escape:
      //m_inputHandler->mouseLock(false);
      //m_Mode = Mode::MENU;
      //Stop();
      return true;
    case eKI_Enter:
      gotoGame();
      return true;
    default:
#ifdef GUI
      return gui->OnInputEvent(event);
#else
      return false;
#endif // GUI
    }
  }
  //TODO: FIXT IT
#if NEED_USE_IT
	case sf::Event::MouseMoved:
  {
    if (!can_drag_vp)
      return true;
    auto w = GET_CVAR("r_cam_w")->GetIVal();
    auto h = GET_CVAR("r_cam_h")->GetIVal();
    auto window = m_pSystem->GetIWindow();

    if (GET_CVAR("show_all_frame_buffer")->GetIVal())
    {
      w *= w / GET_CVAR("r_backbuffer_w")->GetFVal();
      h *= h / GET_CVAR("r_backbuffer_h")->GetFVal();
    }

    mouseDelta = sf::Vector2i(event.mouseMove.x, event.mouseMove.y) - mousePrev;
    mousePrev = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);

    if (
      std::abs(event.mouseMove.x - w) <= 8 && !mousePressed
      && event.mouseMove.y > (m_pRender->GetHeight() - h)
      )
    {
      cursor.loadFromSystem(sf::Cursor::SizeHorizontal);
      canDragViewPortWidth = true;
    }
    else if (canDragViewPortWidth && !mousePressed)
    {
      canDragViewPortWidth = false;
    }
    if (
      std::abs(event.mouseMove.y - (m_pRender->GetHeight() - h)) <= 8
      && event.mouseMove.x < w && !mousePressed
      )
    {
      cursor.loadFromSystem(sf::Cursor::SizeVertical);
      canDragViewPortHeight = true;
    }
    else if (canDragViewPortHeight && !mousePressed)
    {
      canDragViewPortHeight = false;
    }
    if (canDragViewPortHeight && canDragViewPortWidth)
    {
      cursor.loadFromSystem(sf::Cursor::SizeBottomLeftTopRight);
    }
    if (!canDragViewPortHeight && !canDragViewPortWidth && !mousePressed)
    {
      cursor.loadFromSystem(sf::Cursor::Arrow);
    }

    window->setCursor(reinterpret_cast<Cursor*>(&cursor));
    break;
  }
  case sf::Event::MouseButtonPressed:
  {
    if (event.mouseButton.button == sf::Mouse::Button::Left)
    {
      mousePressed = true;
    }
    break;
  }
  case sf::Event::MouseButtonReleased:
  {
    if (event.mouseButton.button == sf::Mouse::Button::Left)
    {
      mousePressed = false;
    }
    break;
  }
  default:
#ifdef GUI
    return gui->OnInputEvent(event);
#else
    return false;
#endif // GUI

    if (canDragViewPortWidth && mousePressed)
    {
      ICVar* w = m_Console->GetCVar("r_cam_w");
      w->Set(w->GetIVal() + mouseDelta.x);
    }
    if (canDragViewPortHeight && mousePressed)
    {
      ICVar* h = m_Console->GetCVar("r_cam_h");
      h->Set(h->GetIVal() - mouseDelta.y);
    }
#endif
    return false;
}

bool CGame::DefaultInputEvent(const SInputEvent& event)
{
  return false;
}

bool CGame::EditInputEvent(const SInputEvent& event)
{
  ////////////////////////
  bool keyPressed = event.deviceType == eIDT_Keyboard && event.state == eIS_Pressed;
  bool control = event.modifiers & eMM_Ctrl;
  bool shift = event.modifiers & eMM_Shift;
  bool alt = event.modifiers & eMM_Alt;
  ////////////////////////
  if (keyPressed)
  {
    IObject* obj = m_World->GetActiveScene()->selectedObject()->second;
    switch (event.keyId)
    {
    case eKI_Escape:
      gotoMenu();
      return true;
    case eKI_I:
      obj->move(Movement::FORWARD);
      return true;
    case eKI_U:
      obj->move(Movement::BACKWARD);
      return true;
    case eKI_J:
      obj->move(Movement::DOWN);
      return true;
    case eKI_K:
      obj->move(Movement::UP);
      return true;
    case eKI_V:
      obj->setVisibility(!obj->visible());
      return true;
    case eKI_Tab:
      if (shift)
      {
        m_World->GetActiveScene()->selectPrevObject();
      }
      else
      {
        m_World->GetActiveScene()->selectNextObject();
      }
      return true;
    default:
      return m_player->OnInputEvent(event);
    }
  }
  else
  {
    return m_player->OnInputEvent(event);
  }

  return false;
}

bool CGame::OnInputEventProxy(const SInputEvent& event)
{
  switch (m_Mode)
  {
  case CGame::FPS:
    if (m_player != nullptr) return FpsInputEvent(event);
    else return false;
  case CGame::MENU:
    return MenuInputEvent(event);
  case CGame::FLY:
    return FlyInputEvent(event);
  case CGame::EDIT:
    return EditInputEvent(event);
  default:
    break;
  }
  return false;
}

bool CGame::ShouldHandleEvent(const SInputEvent& event, bool& retflag)
{
  bool keyPressed = event.deviceType == eIDT_Keyboard && event.state == eIS_Pressed;
  bool control = event.modifiers & eMM_Ctrl;
  bool shift = event.modifiers & eMM_Shift;
  bool alt = event.modifiers & eMM_Alt;
  retflag = true;

  if (keyPressed)
  {
    switch (event.keyId)
    {
    case eKI_Tilde:
      if (m_Console->IsOpened())
        m_Console->ShowConsole(false);
      else
        m_Console->ShowConsole(true);
      return true;
    }
  }

  if (m_Console->IsOpened())
    return false;
  retflag = false;
  return {};
}

void CGame::ProcessPMessages(const char* szMsg)
{
  if (stricmp(szMsg, "Quit") == 0)	// quit message
  {
    m_bUpdateRet = false;
    return;
  }
  else
    if (stricmp(szMsg, "Relaunch") == 0)	// relaunch message
    {
      m_bRelaunch = true;
      m_bUpdateRet = false;
      return;
    }
}

bool CGame::IsInPause()
{
  return m_bInPause;
}

void CGame::Stop()
{
  m_bUpdateRet = false;
}

void CGame::PostRender()
{
}

void CGame::gotoMenu()
{
  m_Mode = MENU;
  m_pInput->ShowCursor(true);
  m_pInput->GrabInput(true);
}

void CGame::gotoFullscreen()
{
  isFullScreen = !isFullScreen;
}


bool CGame::InitScripts()
{
  m_pScriptObjectGame = new CScriptObjectGame();
  m_pScriptObjectGame->InitializeTemplate(m_pScriptSystem);

#if 0
  m_pScriptClient = new CScriptObjectClient();
  m_pScriptClient->InitializeTemplate(m_pScriptSystem);

  m_pScriptServer = new CScriptObjectServer();
  m_pScriptServer->InitializeTemplate(m_pScriptSystem);
#endif

  m_pScriptObjectGame->Init(m_pSystem->GetIScriptSystem(), this);
#if 0
  m_pScriptServer->Init(m_pSystem->GetIScriptSystem(), m_pServer);
  m_pScriptClient->Init(m_pSystem->GetIScriptSystem(), m_pClient);
#endif

  m_pScriptSystem->ExecuteFile("scripts/common.lua", true, false);

  fps = 35.f;
  m_pScriptSystem->ExecuteFile("scripts/game.lua");
  class toogle_viewport_drag : public IConsoleCommand
  {
    CGame* game;
  public:
    toogle_viewport_drag(CGame* game) : game(game) {}
    bool execute(CommandDesc& cd)
    {
      game->can_drag_vp = !game->can_drag_vp;
      return true;
    }
  };

  m_Console->AddCommand("toogle_viewport_drag", new toogle_viewport_drag(this));

  m_pScriptSystem->ExecuteFile("scripts/utils.lua");

  bool retflag;
  bool retval = TestScriptSystem(retflag);
  if (retflag) return retval;
  return true;
}

bool CGame::TestScriptSystem(bool& retflag)
{
  retflag = true;
  m_playerObject = m_pScriptSystem->CreateEmptyObject();
  if (!m_pScriptSystem->GetGlobalValue("player", m_playerObject))
  {
    delete m_playerObject;
    m_pSystem->Log("\002 ERROR: can't find player table ");
    return false;
  }
  const char* name;
  int age;
  m_playerObject->GetValue("name", name);
  m_playerObject->GetValue("age", age);
  m_Console->PrintLine("Player name: %s", name);
  m_Console->PrintLine("Player age: %d", age);

  m_Console->AddCommand(
    "enumd",
    R"(
		local formats = System:EnumDisplayFormats()
		for i=1, #formats do
			Console:PrintLine("["..i.."]".. formats[i].width .. " x " .. formats[i].height .. " x " .. formats[i].bpp)
		end
		Console:PrintLine(%2)
		Console:PrintLine(%1)
		)",
    0,
    "Enum Display formats"
  );

#if 0
  HSCRIPTFUNCTION psina;
  m_playerObject->GetValue("TestChanges", psina);

  m_playerObject->SetValue("name", "Psina");
  //m_pScriptSystem->BeginCall(m_playerObject, "TestChanges");
  IScriptObject* console = nullptr;
  m_pScriptSystem->BeginCall("Console", "PrintLine");
  m_pScriptSystem->PushFuncParam(m_ScriptObjectConsole->GetScriptObject());
  m_pScriptSystem->PushFuncParam("alskdjfa;lsdjf call!!!");
  m_pScriptSystem->EndCall(console);

  m_pScriptSystem->BeginCall("player", "TestChanges");
  m_pScriptSystem->PushFuncParam(m_playerObject);
  m_pScriptSystem->PushFuncParam("Test HSCRIPTFUNCTION call!!!");
  m_pScriptSystem->EndCall(console);

  m_pScriptSystem->BeginCall(m_playerObject, "TestChanges");
  m_pScriptSystem->PushFuncParam(m_playerObject);
  m_pScriptSystem->PushFuncParam("Test lkjakldfj call!!!");
  m_pScriptSystem->EndCall(console);

  HSCRIPTFUNCTION PrintLine = 0;
  console->GetValue("PrintLine", PrintLine);
  m_pScriptSystem->BeginCall(PrintLine);
  m_pScriptSystem->PushFuncParam(console);
  m_pScriptSystem->PushFuncParam("=====================Test HSCRIPTFUNCTION call!!!");
  m_pScriptSystem->EndCall();

  int n;
  m_pScriptSystem->BeginCall(console, "PrintLine");
  m_pScriptSystem->PushFuncParam(console);
  m_pScriptSystem->PushFuncParam("Call by table reference");
  m_pScriptSystem->EndCall(console);

  m_playerObject->GetValue("name", name);
  m_Console->PrintLine("Player name: %s", name);
#endif
  retflag = false;
  return {};
}

void CGame::SaveConfiguration(const char* sSystemCfg, const char* sGameCfg, const char* sProfile)
{
}

void CGame::ReloadScripts()
{
}

bool CGame::GetModuleState(EGameCapability eCap)
{
  return false;
}

void CGame::UpdateDuringLoading()
{
}

IXAreaMgr* CGame::GetAreaManager()
{
  return nullptr;
}

ITagPointManager* CGame::GetTagPointManager()
{
  return nullptr;
}

Object* CGame::OnLoad(Object* object, std::string type)
{
  if (type == "player")
    return new CPlayer(object);
  else if (type == "gameobject")
    return new GameObject(object);
  else return object;
}

