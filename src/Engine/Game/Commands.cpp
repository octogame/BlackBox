#include <BlackBox/Game/CGame.hpp>

class BaseCommand : public IEditCommand
{
protected:
	CGame* game;
	glm::vec3 unpack_vector(std::vector<std::wstring>::iterator it, int size = 3)
	{
			glm::vec3 pos;
			pos[0] = _wtof(it->c_str());
			it++;
			pos[1] = _wtof(it->c_str());
			if (size == 3)
			{
				it++;
				pos[2] = _wtof(it->c_str());
			}
			return pos;
	};
public:
	BaseCommand(CGame *game);
	~BaseCommand();

private:

};

BaseCommand::BaseCommand(CGame *game) : game(game)
{
}

BaseCommand::~BaseCommand()
{
}

class LastCommand : public BaseCommand 
{
public:
	LastCommand(CGame *game);
	~LastCommand();
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		
		bool result = false;
		if (cd.history->size() == 0)
			return false;
		if (cd.args.size() == 1)
		{
			for (int i = cd.history->size() - 1 - _wtoi(cd.args[0].c_str()); i < cd.history->size();)
			{
				result = game->handleCommand(cd.history->back());
			}
		}
		else
		{
			result = game->handleCommand(cd.history->back());
		}
		return result;
	}
};

LastCommand::LastCommand(CGame *game) : BaseCommand(game)
{
}

LastCommand::~LastCommand()
{
}

class ClearCommand : public BaseCommand 
{
public:
	ClearCommand(CGame *game);
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		cd.history->clear();
		return true;
	}
};

ClearCommand::ClearCommand(CGame *game) : BaseCommand(game)
{
}

//*******************************************************
class GotoCommand : public BaseCommand 
{
public:
	GotoCommand(CGame *game);
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		if (cd.args.size() == 1)
		{
			std::wstring mode = cd.args[0];
			if (mode == L"MENU")
				game->gotoMenu();
			else if (mode == L"FLY")
				game->gotoFly();
			else if (mode == L"EDIT")
				game->gotoEdit();
			else if (mode == L"FPS")
				game->gotoGame();
			return true;
		}
		return false;
	}
};

GotoCommand::GotoCommand(CGame *game) : BaseCommand(game)
{
}
//*******************************************************
//*******************************************************
class VsyncCommand : public BaseCommand 
{
public:
	VsyncCommand(CGame *game);
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		if (cd.args.size() == 1)
		{
			sf::RenderWindow *rw = reinterpret_cast<sf::RenderWindow *>(game->getWindow()->getHandle());
			std::wstring mode = cd.args[0];
			if (mode == L"on")
				rw->setVerticalSyncEnabled(true); 
			else if (mode == L"off")
				rw->setVerticalSyncEnabled(false); 
			return true;
		}
		return false;
	}
};

VsyncCommand::VsyncCommand(CGame *game) : BaseCommand(game)
{
}
//*******************************************************
//*******************************************************
class QuitCommand : public BaseCommand 
{
public:
	QuitCommand(CGame *game);
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		if (cd.args.size() == 1)
		{
			game->Stop();
			return true;
		}
	}
};

QuitCommand::QuitCommand(CGame *game) : BaseCommand(game)
{
}
//*******************************************************
class MoveCommand : public BaseCommand 
{
	World* m_World;
public:
	MoveCommand(CGame *game);
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		if (cd.args.size() == 3)
		{
			auto pos = unpack_vector(cd.args.begin()++);
			if (cd.command == L"moveto")
				m_World->getActiveScene()->selectedObject()->moveTo(pos);
			else
			{
				std::wstring dir = cd.command.substr(4);
				if (dir == L"")
				{
					m_World->getActiveScene()->selectedObject()->move(pos);
				}
			}
			return true;
		}
		return false;
	}
};

MoveCommand::MoveCommand(CGame *game) : BaseCommand(game)
{
	m_World = game->getWorld();
}
//*******************************************************
class RotateCommand : public BaseCommand 
{
	World* m_World;
public:
	RotateCommand(CGame *game);
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		auto angle = unpack_vector(cd.args.begin()++);
		if (cd.args.size() == 4)
		{
			m_World->getActiveScene()->selectedObject()->rotate(_wtof(cd.args[0].c_str()), angle);
			return true;
		}
		return false;

	}
};

RotateCommand::RotateCommand(CGame *game) : BaseCommand(game)
{
	m_World = game->getWorld();
}
//*******************************************************
class SelectCommand : public BaseCommand 
{
	World* m_World;
public:
	SelectCommand(CGame *game);
private:
	// Inherited via IEditCommand
	virtual bool execute(CommandDesc& cd) override
	{
		std::string tmp;
		tmp.resize(256);
		int pos = 0;
		int i = 0;
		for (auto ch : cd.args[0])
		{
			pos += std::wctomb((char*)&tmp.data()[i], cd.args[0][i]);
			i++;
		}
		std::string name(tmp.c_str());
		return m_World->getActiveScene()->selectObject(name);
	}
};

SelectCommand::SelectCommand(CGame *game) : BaseCommand(game)
{
	m_World = game->getWorld();
}
//*******************************************************

void CGame::initCommands()
{
	m_Commands[L"last"] = new LastCommand(this);
	m_Commands[L"clear"] = new ClearCommand(this);
	m_Commands[L"goto"] = new GotoCommand(this);
	m_Commands[L"vsync"] = new VsyncCommand(this);
	m_Commands[L"quit"] = new QuitCommand(this);
	m_Commands[L"move"] = new MoveCommand(this);
	m_Commands[L"rotate"] = new RotateCommand(this);
	m_Commands[L"select"] = new SelectCommand(this);
}
