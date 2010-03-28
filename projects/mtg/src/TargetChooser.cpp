#include "../include/config.h"
#include "../include/TargetChooser.h"
#include "../include/CardDescriptor.h"
#include "../include/MTGGameZones.h"
#include "../include/GameObserver.h"
#include "../include/Subtypes.h"
#include "../include/Counters.h"



TargetChooser * TargetChooserFactory::createTargetChooser(string s, MTGCardInstance * card, MTGAbility * ability){
  if (!s.size()) return NULL;

  int zones[10];
  int nbzones = 0;
  size_t found;
  bool other = false;

  found = s.find("mytgt");
  if (found == 0){
    MTGCardInstance * target = card->target;
    if (ability) target = (MTGCardInstance *) (ability->target);
    return NEW CardTargetChooser(target,card);
  };

  found = s.find("other ");
  if (found == 0){
    other = true;
    s=s.substr(6);
  }

  found = s.find("player");
  if (found != string::npos){
    int maxtargets = 1;
    size_t several = s.find_first_of('s',5);
    if (several != string::npos) maxtargets = -1;
    found = s.find("creature");
    if (found != string::npos) return NEW DamageableTargetChooser(card,maxtargets,other); //Any Damageable target (player, creature)
    return NEW PlayerTargetChooser(card,maxtargets); //Any player
  }

  string s1;
  found = s.find("|");
  if (found != string::npos){
    string s2;
    s1 = s.substr(0,found);
    s2 = s.substr(found+1);
    while(s2.size()){
      found = s2.find(",");
      string zoneName;
      if (found != string::npos){
        zoneName = s2.substr(0,found);
        s2 = s2.substr(found+1);
      }else{
        zoneName = s2;
        s2 = "";
      }
      zones[nbzones] = MTGGameZone::MY_BATTLEFIELD;

      if(zoneName.compare("*") == 0){
         zones[nbzones] = MTGGameZone::ALL_ZONES;
      }else if(zoneName.compare("graveyard") == 0){
        zones[nbzones] = MTGGameZone::MY_GRAVEYARD;
        nbzones++;
        zones[nbzones] = MTGGameZone::OPPONENT_GRAVEYARD;
      }else if(zoneName.compare("battlefield") == 0 || zoneName.compare("inplay") == 0){
          zones[nbzones] = MTGGameZone::MY_BATTLEFIELD;
          nbzones++;
          zones[nbzones] = MTGGameZone::OPPONENT_BATTLEFIELD;
      }else if(zoneName.compare("stack") == 0){
        zones[nbzones] = MTGGameZone::MY_STACK;
        nbzones++;
        zones[nbzones] = MTGGameZone::OPPONENT_STACK;
      }else{
          int zone = MTGGameZone::zoneStringToId(zoneName);
          if (zone) zones[nbzones] = zone;
      }
      nbzones++;
    }
  }else{
    s1 = s;
    nbzones = 2;
    zones[0]= MTGGameZone::MY_BATTLEFIELD;
    zones[1]= MTGGameZone::OPPONENT_BATTLEFIELD;
  }

  TargetChooser * tc = NULL;
  int maxtargets = 1;
  CardDescriptor * cd = NULL;

  while(s1.size()){
    found = s1.find(",");
    string typeName;
    if (found != string::npos){
      typeName = s1.substr(0,found);
      s1 = s1.substr(found+1);
    }else{
      typeName = s1;
      s1 = "";
    }

    //Advanced cards caracteristics ?
    found = typeName.find("[");
    if (found != string::npos){
      int nbminuses = 0;
      int end = typeName.find("]");
      string attributes = typeName.substr(found+1,end-found-1);
      cd = NEW CardDescriptor();
      while(attributes.size()){
        size_t found2 = attributes.find(";");
        string attribute;
        if (found2 != string::npos){
          cd->mode = CD_OR;
          attribute = attributes.substr(0,found2);
          attributes = attributes.substr(found2+1);
        }else{
          attribute = attributes;
          attributes = "";
        }
        int minus = 0;
        if (attribute[0] == '-'){
          minus = 1;
          nbminuses++;
          attribute=attribute.substr(1);
        }
        int comparisonMode = COMPARISON_NONE;
        int comparisonCriterion = 0;
        if (attribute.size() > 1){
          size_t operatorPosition = attribute.find("=",1);
          if (operatorPosition != string::npos){
            comparisonCriterion = atoi(attribute.substr(operatorPosition+1,attribute.size()-operatorPosition-1).c_str());
            switch (attribute[operatorPosition-1]){
              case '<':
                if (minus){
                  comparisonMode = COMPARISON_GREATER;
                }else{
                  comparisonMode = COMPARISON_AT_MOST;
                }
                operatorPosition--;
                break;
              case '>':
                if (minus){
                  comparisonMode = COMPARISON_LESS;
                }else{
                  comparisonMode = COMPARISON_AT_LEAST;
                }
                operatorPosition--;
                break;
              default:
                if (minus){
                  comparisonMode = COMPARISON_UNEQUAL;
                }else{
                  comparisonMode = COMPARISON_EQUAL;
                }
            }
            attribute = attribute.substr(0,operatorPosition);
          }
        }

        //Attacker
        if (attribute.find("attacking") != string::npos){
          if (minus){
	          cd->attacker = -1;
          }else{
	          cd->attacker = 1;
          }
          //Blocker
        }else if (attribute.find("blocking") != string::npos){
          if (minus){
            cd->defenser = & MTGCardInstance::NoCard;
          }else{
	          cd->defenser = & MTGCardInstance::AnyCard;
          }
        //Tapped, untapped
        }else if (attribute.find("tapped") != string::npos){
          if (minus){
	          cd->unsecureSetTapped(-1);
          }else{
	          cd->unsecureSetTapped(1);
          }
        //Token
        }else if (attribute.find("token") != string::npos){
          if (minus){
            cd->isToken = -1;
          }else{
	          cd->isToken = 1;
          }
        //Power restrictions
        }else if (attribute.find("power") != string::npos){
          cd->setPower(comparisonCriterion);
          cd->powerComparisonMode = comparisonMode;
        //Toughness restrictions
        }else if (attribute.find("toughness") != string::npos){
          cd->setToughness(comparisonCriterion);
          cd->toughnessComparisonMode = comparisonMode;
        //Manacost restrictions
        }else if (attribute.find("manacost") != string::npos){
          cd->convertedManacost = comparisonCriterion;
          cd->manacostComparisonMode = comparisonMode;
        //Counter Restrictions
        }else if (attribute.find("counter") != string::npos) {
          if (attribute.find("{any}") != string::npos) {
            cd->anyCounter = 1;
          }else{
            size_t start = attribute.find("{");
            size_t end = attribute.find("}");
            string counterString = attribute.substr(start+1,end-start-1);
            AbilityFactory * abf = NEW AbilityFactory();
            Counter * counter = abf->parseCounter(counterString,card);
            if (counter) {
              cd->counterName = counter->name;
              cd->counterNB = counter->nb;
              cd->counterPower = counter->power;
              cd->counterToughness = counter->toughness;
            }
            if (minus) {
              cd->counterComparisonMode = COMPARISON_LESS;
            }else{
              cd->counterComparisonMode = COMPARISON_AT_LEAST;
            }
          }
        }else{
          int attributefound = 0;
          //Colors
          for (int cid = 1; cid < Constants::MTG_NB_COLORS - 1; cid++){ //remove Artifact and Land from the loop
	          if (attribute.find(Constants::MTGColorStrings[cid]) != string::npos){
	            attributefound = 1;
	            if (minus){
	              cd->colors[cid] = -1;
	            }else{
	              cd->colors[cid] = 1;
	            }
	          }
          }
          if (!attributefound){
	          //Abilities
	          for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++){
	            if (attribute.find(Constants::MTGBasicAbilities[j]) != string::npos){
	              attributefound = 1;
	              if (minus){
	                cd->basicAbilities[j] = -1;
	              }else{
	                cd->basicAbilities[j] = 1;
	              }
	            }
	          }
          }

          if (!attributefound){
	          //Subtypes
	          if (minus){
	            cd->setNegativeSubtype(attribute);
	          }else{
	            cd->setSubtype(attribute);
	          }
          }
        }
      }
      if (nbminuses) cd->mode = CD_AND;
      typeName = typeName.substr(0,found);
    }
    //X targets allowed ?
    if (typeName.at(typeName.length()-1) == 's' && !Subtypes::subtypesList->find(typeName,false) && typeName.compare("this")!=0){
      typeName = typeName.substr(0,typeName.length()-1);
      maxtargets = -1;
    }
    if (cd){
      if (!tc){
        if (typeName.compare("*")!=0) cd->setSubtype(typeName);

        tc = NEW DescriptorTargetChooser(cd,zones,nbzones,card,maxtargets,other);
      }else{
        delete(cd);
        return NULL;
      }
    }else{
      if (!tc){
        if (typeName.compare("*")==0){
          return NEW TargetZoneChooser(zones, nbzones,card, maxtargets,other);
        }else if (typeName.compare("this")==0){
          return NEW CardTargetChooser(card,card,zones, nbzones);
        }else{
          tc =  NEW TypeTargetChooser(typeName.c_str(), zones, nbzones, card,maxtargets,other);
        }
      }else{
        ((TypeTargetChooser *)tc)->addType(typeName.c_str());
        tc->maxtargets = maxtargets;
      }
    }
  }
  return tc;
}

TargetChooser * TargetChooserFactory::createTargetChooser(MTGCardInstance * card){
  int id = card->getId();
  string s = card->spellTargetType;
  if (card->alias){
    id = card->alias;
    //TODO load target as well... ?
  }
  TargetChooser * tc = createTargetChooser(s, card);
  if (tc) return tc;
  //Any target than cannot be defined automatically is determined by its id
  switch (id){
    //Spell
  case 1224: //Spell blast
    {
#if defined (WIN32) || defined (LINUX)
      OutputDebugString ("Counter Spell !\n");
#endif
      return NEW SpellTargetChooser(card);
    }
    //Spell Or Permanent
  case 1282: //ChaosLace
  case 1152: //DeathLace
  case 1358: //PureLace
  case 1227: //ThoughLace
  case 1257: //Lifelace
    {
      return NEW SpellOrPermanentTargetChooser(card);
    }
    //Red Spell or Permanent
  case 1191: //Blue Elemental Blast
    {
      return NEW SpellOrPermanentTargetChooser(card,Constants::MTG_COLOR_RED);
    }
    //Blue Spell or Permanent
  case 1312: //Red Elemental Blast
    {
      return NEW SpellOrPermanentTargetChooser(card,Constants::MTG_COLOR_BLUE);
    }
    //Damage History
  case 1344: //Eye for an Eye
    {
      return NEW DamageTargetChooser(card,-1,1,RESOLVED_OK);
    }
  default:
    {
      return NULL;
    }
  }
}


TargetChooser::TargetChooser(MTGCardInstance * card, int _maxtargets, bool _other): TargetsList(){
  forceTargetListReady = 0;
  source = card;
  targetter = card;
  maxtargets = _maxtargets;
  other = _other;
}

//Default targetter : every card can be targetted, unless it is protected from the targetter card
// For spells that do not "target" a specific card, set targetter to NULL
bool TargetChooser::canTarget(Targetable * target){
  if (!target) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    if (other){
      MTGCardInstance * tempcard = card;
      while (tempcard) {
        if (tempcard == source) return false;
        tempcard = tempcard->previous;
      }
    }
    if (source && targetter && card->isInPlay() && (card->has(Constants::SHROUD)|| card->protectedAgainst(targetter) )) return false;
    if (source && targetter && card->isInPlay() && (targetter->controller() != card->controller()) && (card->has(Constants::OPPONENTSHROUD) || card->protectedAgainst(targetter))) return false;
    return true;
  }
  else if (target->typeAsTarget() == TARGET_STACKACTION)
    return true;
  return false;
}


int TargetChooser::addTarget(Targetable * target){
  if (canTarget(target)){
    TargetsList::addTarget(target);
  }

  return targetsReadyCheck();
}


int TargetChooser::ForceTargetListReady(){
  int state =  targetsReadyCheck() ;
  if (state == TARGET_OK){
    forceTargetListReady = 1;
  }
  return forceTargetListReady;
}

int TargetChooser::targetsReadyCheck(){
  if (cursor == 0){
    return TARGET_NOK;
  }
  if (full()){
    return TARGET_OK_FULL;
  }
  if (!ready()){
    return TARGET_OK_NOT_READY;
  }
  return TARGET_OK;
}

int TargetChooser::targetListSet(){
  int state = targetsReadyCheck();
  if (state == TARGET_OK_FULL || forceTargetListReady){
    return 1;
  }
  return 0;
}

bool TargetChooser::validTargetsExist(){
  for (int i = 0; i < 2; ++i){
    Player *p = GameObserver::GetInstance()->players[i];
    if (canTarget(p)) return true;
    MTGGameZone * zones[] = {p->game->inPlay,p->game->graveyard,p->game->hand,p->game->library};
    for (int k = 0; k < 4; k++){
      MTGGameZone * z = zones[k];
      if (targetsZone(z)){
        for (int j = 0; j < z->nb_cards; j++){
          if (canTarget(z->cards[j])) return true;
        }
      }
    }
  }
  return false;
}

/**
  a specific Card
**/
CardTargetChooser::CardTargetChooser(MTGCardInstance * _card, MTGCardInstance * source,int * _zones, int _nbzones):TargetZoneChooser(_zones,_nbzones,source){
  validTarget = _card;
}

bool CardTargetChooser::canTarget(Targetable * target ){
  if (!target) return false;
  if (target->typeAsTarget() != TARGET_CARD) return false;
  if (!nbzones && !TargetChooser::canTarget(target)) return false;
  if (nbzones && !TargetZoneChooser::canTarget(target)) return false;
  MTGCardInstance * card = (MTGCardInstance *) target;
  while (card) {
    if (card == validTarget) return true;
    card = card->previous;
  }
  return false;
}

  CardTargetChooser * CardTargetChooser::clone() const{
    CardTargetChooser * a =  NEW CardTargetChooser(*this);
    return a;
  }

/**
   Choose anything that has a given list of types
**/
TypeTargetChooser::TypeTargetChooser(const char * _type, MTGCardInstance * card, int _maxtargets,bool other):TargetZoneChooser(card, _maxtargets,other){
  int id = Subtypes::subtypesList->find(_type);
  nbtypes = 0;
  addType(id);
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
}

TypeTargetChooser::TypeTargetChooser(const char * _type, int * _zones, int nbzones, MTGCardInstance * card, int _maxtargets,bool other):TargetZoneChooser(card, _maxtargets,other){
  int id = Subtypes::subtypesList->find(_type);
  nbtypes = 0;
  addType(id);
  if (nbzones == 0){
    int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
}

void TypeTargetChooser::addType(const char * _type){
  int id = Subtypes::subtypesList->find(_type);
  addType(id);
}

void TypeTargetChooser::addType(int type){
  types[nbtypes] = type;
  nbtypes++;
}

bool TypeTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    for (int i= 0; i < nbtypes; i++){
      if (card->hasSubtype(types[i])) return true;
      if (Subtypes::subtypesList->find(card->getLCName()) == types[i]) return true;
    }
    return false;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      for (int i= 0; i < nbtypes; i++){
        if (card->hasSubtype(types[i])) return true;
        if (Subtypes::subtypesList->find(card->name) == types[i]) return true;
      }
      return false;
    }
  }
  return false;
}

  TypeTargetChooser *  TypeTargetChooser::clone() const{
     TypeTargetChooser * a =  NEW  TypeTargetChooser(*this);
    return a;
  }


/**
    A Target Chooser associated to a Card Descriptor object, for fine tuning of targets description
**/
DescriptorTargetChooser::DescriptorTargetChooser(CardDescriptor * _cd, MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
  cd = _cd;
}

DescriptorTargetChooser::DescriptorTargetChooser(CardDescriptor * _cd, int * _zones, int nbzones, MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  if (nbzones == 0){
    int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
  cd = _cd;
}

bool DescriptorTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * _target = (MTGCardInstance *) target;
    if (cd->match(_target)) return true;
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      if (cd->match(card)) return true;
    }
  }
  return false;
}

DescriptorTargetChooser::~DescriptorTargetChooser(){
  SAFE_DELETE(cd);
}

 DescriptorTargetChooser *  DescriptorTargetChooser ::clone() const{
     DescriptorTargetChooser  * a =  NEW  DescriptorTargetChooser (*this);
     a->cd = NEW CardDescriptor(*cd);
    return a;
  }


/**
   Choose a creature
**/

CreatureTargetChooser::CreatureTargetChooser( MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
}

CreatureTargetChooser::CreatureTargetChooser(int * _zones, int nbzones, MTGCardInstance * card, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  if (nbzones == 0){
    int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
    init(default_zones,2);
  }else{
    init(_zones, nbzones);
  }
}


bool CreatureTargetChooser::canTarget(Targetable * target){
  if (!TargetZoneChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    return card->isCreature();
  }
  return false;
}

   CreatureTargetChooser *   CreatureTargetChooser::clone() const{
      CreatureTargetChooser * a =  NEW  CreatureTargetChooser(*this);
    return a;
  }


/* TargetzoneChooser targets everything in a given zone */
TargetZoneChooser::TargetZoneChooser(MTGCardInstance * card, int _maxtargets, bool other):TargetChooser(card,_maxtargets, other){
  init(NULL,0);
}

TargetZoneChooser::TargetZoneChooser(int * _zones, int _nbzones,MTGCardInstance * card, int _maxtargets, bool other):TargetChooser(card,_maxtargets, other){
  init(_zones, _nbzones);
}

int TargetZoneChooser::init(int * _zones, int _nbzones){
  for (int i = 0; i < _nbzones; i++){
    zones[i] = _zones[i];
  }
  nbzones = _nbzones;
  return nbzones;
}

int TargetZoneChooser::setAllZones(){
    int zones[] = {
      MTGGameZone::ALL_ZONES, 
    };

    init(zones,1);
  return 1;
} 


bool TargetZoneChooser::canTarget(Targetable * target){
  if (!TargetChooser::canTarget(target)) return false;
  if (target->typeAsTarget() == TARGET_CARD){
    MTGCardInstance * card = (MTGCardInstance *) target;
    for (int i = 0; i<nbzones; i++){
      if (zones[i] == MTGGameZone::ALL_ZONES) return true;
      if (MTGGameZone::intToZone(zones[i],source,card)->hasCard(card)) return true;
    }
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
OutputDebugString ("CHECKING INTERRUPTIBLE\n");
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      MTGCardInstance * card = spell->source;
      for (int i = 0; i<nbzones; i++)
	if (MTGGameZone::intToZone(zones[i],source,card)->hasCard(card)) return true;
    }
  }
  return false;
}


bool TargetZoneChooser::targetsZone(MTGGameZone * z){
  for (int i = 0; i < nbzones; i++)
    if (MTGGameZone::intToZone(zones[i],source) == z) return true;
  return false;
}

    TargetZoneChooser *    TargetZoneChooser::clone() const{
      TargetZoneChooser * a =  NEW   TargetZoneChooser(*this);
    return a;
  }

/* Player Target */
PlayerTargetChooser::PlayerTargetChooser(MTGCardInstance * card, int _maxtargets, Player *p):TargetChooser(card, _maxtargets), p(p){
}

bool PlayerTargetChooser::canTarget(Targetable * target){
  return (target->typeAsTarget() == TARGET_PLAYER) && (!p || p == (Player*)target);
}

    PlayerTargetChooser*   PlayerTargetChooser::clone() const{
      PlayerTargetChooser * a =  NEW   PlayerTargetChooser(*this);
    return a;
  }


/*Damageable Target */
bool DamageableTargetChooser::canTarget(Targetable * target){
  if (target->typeAsTarget() == TARGET_PLAYER){
    return true;
  }
  return CreatureTargetChooser::canTarget(target);
}

    DamageableTargetChooser*   DamageableTargetChooser::clone() const{
      DamageableTargetChooser * a =  NEW   DamageableTargetChooser(*this);
    return a;
  }

/*Spell */



SpellTargetChooser::SpellTargetChooser(MTGCardInstance * card,int _color, int _maxtargets, bool other ):TargetChooser(card, _maxtargets, other){
  color = _color;
}

bool SpellTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      card = spell->source;
      if (card && (color == -1 || card->hasColor(color))) return true;
    }
  }

  return false;
}

    SpellTargetChooser*   SpellTargetChooser::clone() const{
      SpellTargetChooser * a =  NEW   SpellTargetChooser(*this);
    return a;
  }

/*Spell or Permanent */
SpellOrPermanentTargetChooser::SpellOrPermanentTargetChooser(MTGCardInstance * card,int _color, int _maxtargets, bool other):TargetZoneChooser(card, _maxtargets, other){
  int default_zones[] = {MTGGameZone::MY_BATTLEFIELD, MTGGameZone::OPPONENT_BATTLEFIELD};
  init(default_zones,2);
  color = _color;
}

bool SpellOrPermanentTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_CARD){
    card = (MTGCardInstance *) target;
    if (color == -1 || card->hasColor(color)) return TargetZoneChooser::canTarget(target);
  }else if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_SPELL && action->state==NOT_RESOLVED){
      Spell * spell = (Spell *) action;
      card = spell->source;
      if (card && (color == -1 || card->hasColor(color))) return true;
    }
  }
  return false;
}

    SpellOrPermanentTargetChooser*   SpellOrPermanentTargetChooser::clone() const{
      SpellOrPermanentTargetChooser * a =  NEW   SpellOrPermanentTargetChooser(*this);
    return a;
  }

/*Damage */
DamageTargetChooser::DamageTargetChooser(MTGCardInstance * card,int _color, int _maxtargets, int _state):TargetChooser(card, _maxtargets){
  color = _color;
  state = _state;
}

bool DamageTargetChooser::canTarget(Targetable * target){
  MTGCardInstance * card = NULL;
  if (target->typeAsTarget() == TARGET_STACKACTION){
    Interruptible * action = (Interruptible *) target;
    if (action->type == ACTION_DAMAGE && (action->state == state || state == -1)){
      Damage * damage = (Damage *) action;
      card = damage->source;
      if (card && (color == -1 || card->hasColor(color))) return true;
    }
  }
  return false;
}

    DamageTargetChooser*   DamageTargetChooser::clone() const{
     DamageTargetChooser * a =  NEW   DamageTargetChooser(*this);
    return a;
  }
