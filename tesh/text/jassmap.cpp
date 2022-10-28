#include <text/jassmap.h>

//Jass语法折叠关键词
const char* JassKeywords = "globals endglobals library endlibrary scope endscope module endmodule "
		"struct endstruct interface endinterface function endfunction method endmethod if endif loop endloop "
		"";
//Jass主要关键词
const char* JassKeywords2 = "native set local call takes returns return "
							"exitwhen then elseif else and  or not "
							"private public readonly debug keyword key allocate deallocate onInit onDestroy extends stub hook "
							"initializer requires uses needs static  implement optional operator initializer constant type"
							"";
//分类关键词
const char* JassKeywords3 = "null true false this"
							"";
//类型关键词
const char* JassTypes = "array thistype int bool nothing boolean integer real string code handle agent event player widget unit destructable item ability buff force group trigger triggercondition triggeraction "
	"timer location region rect boolexpr sound conditionfunc filterfunc unitpool itempool race alliancetype racepreference "
	"gamestate igamestate fgamestate playerstate playerscore playergameresult unitstate aidifficulty eventid gameevent playerevent "
	"playerunitevent unitevent limitop widgetevent dialogevent unittype gamespeed gamedifficulty gametype mapflag mapvisibility mapsetting "
	"mapdensity mapcontrol playerslotstate volumegroup camerafield camerasetup playercolor placement startlocprio raritycontrol blendmode "
	"texmapflags effect effecttype weathereffect terraindeformation fogstate fogmodifier dialog button quest questitem defeatcondition timerdialog "
	"leaderboard multiboard multiboarditem trackable gamecache version itemtype texttag attacktype damagetype weapontype soundtype lightning pathingtype "
	"image ubersplat hashtable"
	"";
//Lua关键词
const char* LuaKeywords = "if do function repeat end elseif until "
"an or not table nil for while break in return goto true false then elseif else local";

const char* JassCjKeywords = ""; 
const char* JassBjKeywords = "";
const char* JassConstantKeywords = "";
