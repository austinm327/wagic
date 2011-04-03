#ifndef _GAME_STATE_DUEL_H_
#define _GAME_STATE_DUEL_H_

#include "GameState.h"
#include "SimpleMenu.h"
#include "SimplePopup.h"
#include "DeckMenu.h"
#include "MTGDeck.h"
#include "GameObserver.h"

#define CHOOSE_OPPONENT 7

#ifdef TESTSUITE
class TestSuite;
#endif
class Credits;
class Rules;
#ifdef NETWORK_SUPPORT
class JNetwork;
#endif

class GameStateDuel: public GameState, public JGuiListener
{
private:
#ifdef TESTSUITE
    TestSuite * testSuite;
#endif

    Credits * credits;
    int mGamePhase;
    Player * mCurrentPlayer;
    Player * mPlayers[2];
    GameObserver * game;
    DeckMenu * deckmenu;
    DeckMenu * opponentMenu;
    SimpleMenu * menu;
    SimplePopup * popupScreen; // used for informational screens, modal
    static int selectedPlayerDeckId;
    static int selectedAIDeckId;

    bool premadeDeck;
    int OpponentsDeckid;
    string musictrack;
    Rules * rules;

    bool MusicExist(string FileName);
    void loadPlayer(int playerId, int decknb = 0, bool isAI = false, bool isNetwork = false);
    void ConstructOpponentMenu(); //loads the opponentMenu if it doesn't exist
    void initScroller();

public:
    GameStateDuel(GameApp* parent);
    virtual ~GameStateDuel();
#ifdef TESTSUITE
    void loadTestSuitePlayers();
#endif
    virtual void ButtonPressed(int ControllerId, int ControlId);
    virtual void Start();
    virtual void End();
    virtual void Update(float dt);
    virtual void Render();
    void initRand(unsigned seed = 0);

    enum ENUM_DUEL_STATE_MENU_ITEM
    {
        MENUITEM_CANCEL = kCancelMenuID,
        MENUITEM_NEW_DECK = -10,
        MENUITEM_RANDOM_PLAYER = -11,
        MENUITEM_RANDOM_AI = -12,
        MENUITEM_MAIN_MENU = -13,
        MENUITEM_EVIL_TWIN = -14,
        MENUITEM_MULLIGAN = -15,
#ifdef NETWORK_SUPPORT
        MENUITEM_REMOTE_CLIENT = -16,
        MENUITEM_REMOTE_SERVER = -17,
#endif
        MENUITEM_MORE_INFO = kInfoMenuID
    };

};

#endif
