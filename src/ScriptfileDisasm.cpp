// ======================= ZoneTool =======================
// zonetool, a fastfile linker for various
// Call of Duty titles. 
//
// Project: https://github.com/ZoneTool/gsc-asm
// Author: RektInator (https://github.com/RektInator)
// License: GNU GPL v3.0
// ========================================================
#include "stdafx.hpp"

long flength(FILE* fp)
{
	long i = ftell(fp);
	fseek(fp, 0, SEEK_END);
	long ret = ftell(fp);
	fseek(fp, i, SEEK_SET);
	return ret;
}

template <typename ... Args> std::string va(const std::string& format, Args ... args)
{
	size_t size = _snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	std::vector < char > buf;
	buf.resize(size);
	_snprintf(buf.data(), size, format.c_str(), args ...);
	return std::string(buf.data(), buf.data() + size - 1);
}

class ByteBuffer
{
private:
	std::vector < std::uint8_t >	_data;
	std::size_t						_pos;
	std::size_t						_func_start;

public:
	ByteBuffer(std::string filename)
	{
		_pos = 0;

		FILE* fp = fopen(filename.c_str(), "rb");
		if (fp)
		{
			long len = flength(fp);
			_data.resize(len);
			fread(_data.data(), len, 1, fp);
			fclose(fp);
		}
		else
		{
			printf("Couldn't open file %s!\n", filename.data());
			std::exit(-1);
		}
	}

	bool is_avail()
	{
		if (_pos < _data.size()) return true;
		return false;
	}

	void ParseOP()
	{
		_func_start = _pos;
	}

	void Seek(std::size_t pos)
	{
		_pos += pos;
	}
	void SeekNeg(std::size_t pos)
	{
		_pos -= pos;
	}

	template <typename T> T Read()
	{
		auto ret = *reinterpret_cast<T*>(_data.data() + _pos);
		_pos += sizeof(T);
		return ret;
	}

	std::string ReadString()
	{
		auto ret = std::string(reinterpret_cast<const char*>(_data.data() + _pos));
		_pos += ret.size() + 1;
		return ret;
	}

	std::string ReadOpaqueString()
	{
		auto temp = this->Read<std::uint16_t>();

		if (!temp)
		{
			return this->ReadString();
		}

		return std::to_string(temp);
	}

	std::string GetConsumedBytes()
	{
		std::string shit = "";
		for (int i = _func_start - 1; i < _pos; i++)
		{
			shit = va("%s 0x%02X", shit.data(), (*reinterpret_cast<std::uint8_t*>(_data.data() + i)));
		}
		return shit;
	}

	~ByteBuffer()
	{
		_data.clear();
		_pos = 0;
	}
};

std::unordered_map < std::uint16_t, std::string > builtinmap =
{
	{ 1, "getweaponindexforname" },
	{ 2, "target_getarray" },
	{ 3, "ban" },
	{ 14, "setprintchannel" },
	{ 15, "print" },
	{ 16, "println" },
	{ 17, "print3d" },
	{ 18, "line" },
	{ 19, "spawnturret" },
	{ 20, "canspawnturret" },
	{ 21, "assert" },
	{ 38, "assertex" },
	{ 39, "assertmsg" },
	{ 40, "isdefined" },
	{ 41, "isstring" },
	{ 42, "setdvar" },
	{ 43, "setdynamicdvar" },
	{ 44, "setdvarifuninitialized" },
	{ 45, "setdevdvar" },
	{ 46, "setdevdvarifuninitialized" },
	{ 47, "getdvar" },
	{ 48, "getdvarint" },
	{ 49, "getdvarfloat" },
	{ 50, "getdvarvector" },
	{ 51, "gettime" },
	{ 52, "getentbynum" },
	{ 53, "getweaponmodel" },
	{ 57, "setsunlight" },
	{ 58, "resetsunlight" },
	{ 81, "getweaponhidetags" },
	{ 82, "getanimlength" },
	{ 83, "animhasnotetrack" },
	{ 84, "getnotetracktimes" },
	{ 85, "spawn" },
	{ 86, "spawnloopsound" },
	{ 87, "paramerror" },
	{ 88, "bullettrace" },
	{ 116, "sighttracepassed" },
	{ 117, "physicstrace" },
	{ 118, "physicstracenormal" },
	{ 119, "playerphysicstrace" },
	{ 120, "getgroundposition" },
	{ 121, "getmovedelta" },
	{ 122, "getangledelta" },
	{ 123, "getnorthyaw" },
	{ 150, "setnorthyaw" },
	{ 151, "setslowmotion" },
	{ 152, "randomint" },
	{ 153, "randomfloat" },
	{ 154, "randomintrange" },
	{ 155, "randomfloatrange" },
	{ 156, "sin" },
	{ 157, "cos" },
	{ 158, "tan" },
	{ 159, "asin" },
	{ 160, "acos" },
	{ 161, "atan" },
	{ 162, "castint" },
	{ 163, "castfloat" },
	{ 164, "abs" },
	{ 165, "min" },
	{ 198, "max" },
	{ 199, "floor" },
	{ 200, "ceil" },
	{ 201, "exp" },
	{ 202, "log" },
	{ 203, "sqrt" },
	{ 204, "squared" },
	{ 205, "clamp" },
	{ 206, "angleclamp" },
	{ 207, "angleclamp180" },
	{ 208, "vectorfromlinetopoint" },
	{ 209, "pointonsegmentnearesttopoint" },
	{ 210, "distance" },
	{ 211, "distance2d" },
	{ 212, "distancesquared" },
	{ 213, "length" },
	{ 214, "lengthsquared" },
	{ 215, "closer" },
	{ 216, "vectordot" },
	{ 217, "visionsetthermal" },
	{ 218, "visionsetpain" },
	{ 219, "endlobby" },
	{ 220, "setac130ambience" },
	{ 221, "getmapcustom" },
	{ 222, "updateskill" },
	{ 223, "spawnsighttrace" },
	{ 224, "incrementcounter" },
	{ 225, "getcountertotal" },
	{ 246, "vectornormalize" },
	{ 247, "vectortoangles" },
	{ 248, "vectortoyaw" },
	{ 249, "vectorlerp" },
	{ 250, "anglestoup" },
	{ 251, "anglestoright" },
	{ 252, "anglestoforward" },
	{ 253, "combineangles" },
	{ 254, "transformmove" },
	{ 255, "issubstr" },
	{ 256, "isendstr" },
	{ 257, "getsubstr" },
	{ 258, "tolower" },
	{ 259, "strtok" },
	{ 260, "stricmp" },
	{ 261, "ambientplay" },
	{ 262, "getuavstrengthmax" },
	{ 263, "getuavstrengthlevelneutral" },
	{ 264, "getuavstrengthlevelshowenemyfastsweep" },
	{ 265, "getuavstrengthlevelshowenemydirectional" },
	{ 266, "blockteamradar" },
	{ 267, "unblockteamradar" },
	{ 268, "isteamradarblocked" },
	{ 269, "getassignedteam" },
	{ 270, "setmatchdata" },
	{ 271, "getmatchdata" },
	{ 272, "sendmatchdata" },
	{ 273, "clearmatchdata" },
	{ 274, "setmatchdatadef" },
	{ 275, "setmatchclientip" },
	{ 276, "setmatchdataid" },
	{ 277, "setclientmatchdata" },
	{ 278, "getclientmatchdata" },
	{ 279, "setclientmatchdatadef" },
	{ 280, "sendclientmatchdata" },
	{ 281, "getbuildversion" },
	{ 282, "getbuildnumber" },
	{ 283, "getsystemtime" },
	{ 284, "getmatchrulesdata" },
	{ 285, "isusingmatchrulesdata" },
	{ 286, "kick" },
	{ 287, "issplitscreen" },
	{ 288, "setmapcenter" },
	{ 289, "setgameendtime" },
	{ 290, "visionsetnaked" },
	{ 291, "visionsetnight" },
	{ 292, "visionsetmissilecam" },
	{ 293, "ambientstop" },
	{ 294, "precachemodel" },
	{ 295, "precacheshellshock" },
	{ 296, "precacheitem" },
	{ 297, "precacheshader" },
	{ 298, "precachestring" },
	{ 299, "precachemenu" },
	{ 300, "precacherumble" },
	{ 301, "precachelocationselector" },
	{ 302, "precacheleaderboards" },
	{ 303, "loadfx" },
	{ 304, "playfx" },
	{ 305, "playfxontag" },
	{ 306, "stopfxontag" },
	{ 307, "playloopedfx" },
	{ 308, "spawnfx" },
	{ 309, "triggerfx" },
	{ 310, "playfxontagforclients" },
	{ 311, "setwinningteam" },
	{ 312, "announcement" },
	{ 313, "clientannouncement" },
	{ 314, "getteamscore" },
	{ 315, "setteamscore" },
	{ 316, "setclientnamemode" },
	{ 317, "updateclientnames" },
	{ 318, "getteamplayersalive" },
	{ 319, "logprint" },
	{ 320, "worldentnumber" },
	{ 321, "obituary" },
	{ 322, "positionwouldtelefrag" },
	{ 323, "canspawn" },
	{ 324, "getstarttime" },
	{ 325, "precachestatusicon" },
	{ 326, "precacheheadicon" },
	{ 327, "precacheminimapicon" },
	{ 328, "precachempanim" },
	{ 329, "map_restart" },
	{ 330, "exitlevel" },
	{ 331, "addtestclient" },
	{ 332, "makedvarserverinfo" },
	{ 333, "setarchive" },
	{ 334, "allclientsprint" },
	{ 335, "clientprint" },
	{ 336, "mapexists" },
	{ 337, "isvalidgametype" },
	{ 338, "matchend" },
	{ 339, "setplayerteamrank" },
	{ 340, "endparty" },
	{ 341, "setteamradar" },
	{ 342, "getteamradar" },
	{ 343, "setteamradarstrength" },
	{ 344, "getteamradarstrength" },
	{ 345, "getuavstrengthmin" },
	{ 346, "physicsexplosionsphere" },
	{ 347, "physicsexplosioncylinder" },
	{ 348, "physicsjolt" },
	{ 349, "physicsjitter" },
	{ 350, "setexpfog" },
	{ 351, "isexplosivedamagemod" },
	{ 352, "radiusdamage" },
	{ 353, "setplayerignoreradiusdamage" },
	{ 354, "glassradiusdamage" },
	{ 355, "earthquake" },
	{ 356, "getnumparts" },
	{ 357, "objective_onentity" },
	{ 358, "objective_team" },
	{ 359, "objective_player" },
	{ 360, "objective_playerteam" },
	{ 361, "objective_playerenemyteam" },
	{ 362, "iprintln" },
	{ 363, "iprintlnbold" },
	{ 364, "logstring_0" },
	{ 365, "getent" },
	{ 366, "getentarray" },
	{ 367, "spawnplane" },
	{ 368, "spawnstruct" },
	{ 369, "spawnhelicopter" },
	{ 370, "isalive" },
	{ 371, "isvehicle" },
	{ 372, "createattractorent" },
	{ 373, "createattractororigin" },
	{ 374, "createrepulsorent" },
	{ 375, "createrepulsororigin" },
	{ 376, "deleteattractor" },
	{ 377, "playsoundatpos" },
	{ 378, "newhudelem" },
	{ 379, "newclienthudelem" },
	{ 380, "newteamhudelem" },
	{ 381, "resettimeout" },
	{ 382, "precachefxteamthermal" },
	{ 383, "isplayer" },
	{ 384, "isplayernumber" },
	{ 385, "setwinningplayer" },
	{ 386, "getpartname" },
	{ 387, "weaponfiretime" },
	{ 388, "weaponclipsize" },
	{ 389, "weaponisauto" },
	{ 390, "weaponissemiauto" },
	{ 391, "weaponisboltaction" },
	{ 392, "weaponinheritsperks" },
	{ 393, "weaponburstcount" },
	{ 394, "weapontype" },
	{ 395, "weaponclass" },
	{ 396, "getnextarraykey" },
	{ 397, "sortbydistance" },
	{ 398, "tablelookup" },
	{ 399, "tablelookupbyrow" },
	{ 400, "tablelookupistring" },
	{ 401, "tablelookupistringbyrow" },
	{ 402, "tablelookuprownum" },
	{ 403, "getmissileowner" },
	{ 404, "magicbullet" },
	{ 405, "getweaponflashtagname" },
	{ 406, "averagepoint" },
	{ 407, "averagenormal" },
	{ 408, "vehicle_getspawnerarray" },
	{ 409, "playrumbleonposition" },
	{ 410, "playrumblelooponposition" },
	{ 411, "stopallrumbles" },
	{ 412, "soundexists" },
	{ 413, "openfile" },
	{ 414, "closefile" },
	{ 415, "fprintln" },
	{ 416, "fprintfields" },
	{ 417, "freadln" },
	{ 418, "fgetarg" },
	{ 419, "setminimap" },
	{ 420, "setthermalbodymaterial" },
	{ 421, "getarraykeys" },
	{ 422, "getfirstarraykey" },
	{ 423, "getglass" },
	{ 424, "getglassarray" },
	{ 425, "getglassorigin" },
	{ 426, "isglassdestroyed" },
	{ 427, "destroyglass" },
	{ 428, "deleteglass" },
	{ 429, "getentchannelscount" },
	{ 430, "getentchannelname" },
	{ 431, "objective_add" },
	{ 432, "objective_delete" },
	{ 433, "objective_state" },
	{ 434, "objective_icon" },
	{ 435, "objective_position" },
	{ 436, "objective_current" },
	{ 437, "weaponinventorytype" },
	{ 438, "weaponstartammo" },
	{ 439, "weaponmaxammo" },
	{ 440, "weaponaltweaponname" },
	{ 441, "isweaponcliponly" },
	{ 442, "isweapondetonationtimed" },
	{ 443, "weaponhasthermalscope" },
	{ 444, "getvehiclenode" },
	{ 445, "getvehiclenodearray" },
	{ 446, "getallvehiclenodes" },
	{ 447, "getnumvehicles" },
	{ 448, "precachevehicle" },
	{ 449, "spawnvehicle" },
	{ 450, "vehicle_getarray" },
};
std::string GetBuiltinFuncName(std::uint16_t id)
{
	if (builtinmap.find(id) != builtinmap.end())
		return builtinmap[id];

	return "";
}

std::unordered_map < std::uint16_t, std::string > builtinmethodmap =
{
	{ 32768, "thermaldrawdisable" },
	{ 32770, "helicopter_setdamagestate" },
	{ 32771, "playsoundtoteam" },
	{ 32772, "playsoundtoplayer" },
	{ 32773, "playerhide" },
	{ 32774, "showtoplayer" },
	{ 32775, "enableplayeruse" },
	{ 32776, "disableplayeruse" },
	{ 32777, "makescrambler" },
	{ 32778, "makeportableradar" },
	{ 32779, "maketrophysystem" },
	{ 32780, "placespawnpoint" },
	{ 32781, "setteamfortrigger" },
	{ 32782, "clientclaimtrigger" },
	{ 32783, "clientreleasetrigger" },
	{ 32784, "releaseclaimedtrigger" },
	{ 32785, "isusingonlinedataoffline" },
	{ 32786, "getrestedtime" },
	{ 32787, "send73command_unk" },
	{ 32788, "isonladder" },
	{ 32789, "getcorpseanim" },
	{ 32790, "playerforcedeathanim" },
	{ 32791, "attach" },
	{ 32792, "attachshieldmodel" },
	{ 32798, "startragdoll" },
	{ 32800, "sendleaderboards" },
	{ 32803, "thermaldrawenable" },
	{ 32804, "detach" },
	{ 32805, "detachshieldmodel" },
	{ 32806, "moveshieldmodel" },
	{ 32807, "detachall" },
	{ 32808, "getattachsize" },
	{ 32809, "getattachmodelname" },
	{ 32810, "getattachtagname" },
	{ 32835, "getattachignorecollision" },
	{ 32836, "hidepart" },
	{ 32837, "hidepart_allinstances" },
	{ 32838, "hideallparts" },
	{ 32839, "showpart" },
	{ 32840, "showallparts" },
	{ 32841, "linkto" },
	{ 32842, "linktoblendtotag" },
	{ 32843, "unlink" },
	{ 32844, "setnormalhealth" },
	{ 32847, "show" },
	{ 32848, "hide" },
	{ 32864, "setmode" },
	{ 32865, "getmode" },
	{ 32867, "islinked" },
	{ 32868, "enablelinkto" },
	{ 32878, "playsoundasmaster" },
	{ 32879, "playloopsound" },
	{ 32884, "getnormalhealth" },
	{ 32885, "playerlinkto" },
	{ 32886, "playerlinktodelta" },
	{ 32887, "playerlinkweaponviewtodelta" },
	{ 32888, "playerlinktoabsolute" },
	{ 32889, "playerlinktoblend" },
	{ 32890, "playerlinkedoffsetenable" },
	{ 32891, "setwaypointedgestyle_secondaryarrow" },
	{ 32892, "setwaypointiconoffscreenonly" },
	{ 32893, "fadeovertime" },
	{ 32894, "scaleovertime" },
	{ 32895, "moveovertime" },
	{ 32896, "reset" },
	{ 32897, "destroy" },
	{ 32898, "setpulsefx" },
	{ 32899, "setplayernamestring" },
	{ 32900, "changefontscaleovertime" },
	{ 32910, "getorigin" },
	{ 32914, "useby" },
	{ 32915, "playsound" },
	{ 32916, "playerlinkedoffsetdisable" },
	{ 32917, "playerlinkedsetviewznear" },
	{ 32918, "playerlinkedsetusebaseangleforviewclamp" },
	{ 32919, "lerpviewangleclamp" },
	{ 32920, "setviewangleresistance" },
	{ 32921, "geteye" },
	{ 32922, "istouching" },
	{ 32923, "stoploopsound" },
	{ 32924, "stopsounds" },
	{ 32925, "playrumbleonentity" },
	{ 32926, "playrumblelooponentity" },
	{ 32927, "stoprumble" },
	{ 32928, "delete" },
	{ 32929, "setmodel" },
	{ 32930, "laseron" },
	{ 32931, "laseroff" },
	{ 32932, "laseraltviewon" },
	{ 32933, "laseraltviewoff" },
	{ 32934, "thermalvisionon" },
	{ 32935, "thermalvisionoff" },
	{ 32936, "thermalvisionfofoverlayon" },
	{ 32937, "thermalvisionfofoverlayoff" },
	{ 32938, "autospotoverlayon" },
	{ 32939, "autospotoverlayoff" },
	{ 32940, "setcontents" },
	{ 32941, "makeusable" },
	{ 32942, "makeunusable" },
	{ 32950, "settext" },
	{ 32951, "clearalltextafterhudelem" },
	{ 32952, "setshader" },
	{ 32953, "settargetent_hud" },
	{ 32954, "cleartargetent" },
	{ 32955, "settimer" },
	{ 32956, "settimerup" },
	{ 32957, "settimerstatic" },
	{ 32958, "settenthstimer" },
	{ 32959, "settenthstimerup" },
	{ 32960, "settenthstimerstatic" },
	{ 32961, "setclock" },
	{ 32962, "setclockup" },
	{ 32963, "setvalue" },
	{ 32964, "setwaypoint" },
	{ 32965, "setwaypointedgestyle_rotatingicon" },
	{ 32966, "setcursorhint" },
	{ 32967, "sethintstring" },
	{ 32968, "forceusehinton" },
	{ 32969, "forceusehintoff" },
	{ 32970, "makesoft" },
	{ 32971, "makehard" },
	{ 32972, "willneverchange" },
	{ 32973, "startfiring" },
	{ 32974, "stopfiring" },
	{ 32975, "isfiringturret" },
	{ 32976, "startbarrelspin" },
	{ 32977, "stopbarrelspin" },
	{ 32978, "getbarrelspinrate" },
	{ 32979, "remotecontrolturret" },
	{ 32980, "remotecontrolturretoff" },
	{ 32981, "shootturret" },
	{ 32982, "getturretowner" },
	{ 33006, "setsentryowner" },
	{ 33007, "setsentrycarrier" },
	{ 33008, "setturretminimapvisible" },
	{ 33009, "settargetentity" },
	{ 33010, "snaptotargetentity" },
	{ 33011, "cleartargetentity" },
	{ 33012, "getturrettarget" },
	{ 33013, "setplayerspread" },
	{ 33014, "setaispread" },
	{ 33015, "setsuppressiontime" },
	{ 33049, "setconvergencetime" },
	{ 33050, "setconvergenceheightpercent" },
	{ 33051, "setturretteam" },
	{ 33052, "maketurretsolid" },
	{ 33053, "maketurretoperable" },
	{ 33054, "maketurretinoperable" },
	{ 33082, "setturretaccuracy" },
	{ 33083, "setrightarc" },
	{ 33084, "setleftarc" },
	{ 33085, "settoparc" },
	{ 33086, "setbottomarc" },
	{ 33087, "setautorotationdelay" },
	{ 33088, "setdefaultdroppitch" },
	{ 33089, "restoredefaultdroppitch" },
	{ 33090, "turretfiredisable" },
	{ 33121, "turretfireenable" },
	{ 33122, "setturretmodechangewait" },
	{ 33123, "usetriggerrequirelookat" },
	{ 33124, "getstance" },
	{ 33125, "setstance" },
	{ 33126, "itemweaponsetammo" },
	{ 33127, "getammocount" },
	{ 33128, "gettagorigin" },
	{ 33129, "gettagangles" },
	{ 33130, "shellshock" },
	{ 33131, "stunplayer" },
	{ 33132, "stopshellshock" },
	{ 33133, "fadeoutshellshock" },
	{ 33134, "setdepthoffield" },
	{ 33135, "setviewmodeldepthoffield" },
	{ 33136, "setmotionblurmovescale" },
	{ 33168, "setmotionblurturnscale" },
	{ 33169, "setmotionblurzoomscale" },
	{ 33170, "viewkick" },
	{ 33171, "localtoworldcoords" },
	{ 33172, "getentitynumber" },
	{ 33173, "getentityvelocity" },
	{ 33174, "enablegrenadetouchdamage" },
	{ 33175, "disablegrenadetouchdamage" },
	{ 33176, "enableaimassist" },
	{ 33207, "disableaimassist" },
	{ 33208, "radiusdamage" },
	{ 33209, "detonate" },
	{ 33210, "damageconetrace" },
	{ 33211, "sightconetrace" },
	{ 33212, "settargetent" },
	{ 33213, "settargetpos" },
	{ 33214, "cleartarget" },
	{ 33215, "setflightmodedirect" },
	{ 33216, "setflightmodetop" },
	{ 33217, "getlightintensity" },
	{ 33218, "setlightintensity" },
	{ 33219, "isragdoll" },
	{ 33220, "setmovespeedscale" },
	{ 33221, "cameralinkto" },
	{ 33222, "cameraunlink" },
	{ 33251, "controlslinkto" },
	{ 33252, "controlsunlink" },
	{ 33253, "makevehiclesolidcapsule" },
	{ 33254, "makevehiclesolidsphere" },
	{ 33256, "remotecontrolvehicle" },
	{ 33257, "remotecontrolvehicleoff" },
	{ 33258, "isfiringvehicleturret" },
	{ 33259, "drivevehicleandcontrolturret" },
	{ 33260, "drivevehicleandcontrolturretoff" },
	{ 33261, "getplayersetting" },
	{ 33262, "getlocalplayerprofiledata" },
	{ 33263, "setlocalplayerprofiledata" },
	{ 33264, "remotecamerasoundscapeon" },
	{ 33265, "remotecamerasoundscapeoff" },
	{ 33266, "radarjamon" },
	{ 33267, "radarjamoff" },
	{ 33268, "setmotiontrackervisible" },
	{ 33269, "getmotiontrackervisible" },
	{ 33270, "worldpointinreticle_circle" },
	{ 33271, "getpointinbounds" },
	{ 33272, "transfermarkstonewscriptmodel" },
	{ 33273, "setwatersheeting" },
	{ 33274, "setweaponhudiconoverride" },
	{ 33275, "getweaponhudiconoverride" },
	{ 33276, "setempjammed" },
	{ 33277, "playersetexpfog" },
	{ 33278, "isitemunlocked" },
	{ 33279, "getplayerdata" },
	{ 33280, "vehicleturretcontroloff" },
	{ 33281, "isturretready" },
	{ 33282, "vehicledriveto" },
	{ 33283, "dospawn" },
	{ 33284, "isphysveh" },
	{ 33285, "phys_crash" },
	{ 33286, "phys_launch" },
	{ 33287, "phys_disablecrashing" },
	{ 33288, "phys_enablecrashing" },
	{ 33289, "phys_setspeed" },
	{ 33290, "phys_setconveyerbelt" },
	{ 33291, "freehelicopter" },
	{ 33306, "setplayerdata" },
	{ 33307, "trackerupdate" },
	{ 33308, "pingplayer" },
	{ 33309, "buttonpressed" },
	{ 33310, "sayall" },
	{ 33311, "sayteam" },
	{ 33312, "showscoreboard" },
	{ 33313, "setspawnweapon" },
	{ 33314, "dropitem" },
	{ 33315, "dropscavengerbag" },
	{ 33316, "setjitterparams" },
	{ 33317, "sethoverparams" },
	{ 33318, "joltbody" },
	{ 33319, "freevehicle" },
	{ 33320, "getwheelsurface" },
	{ 33321, "getvehicleowner" },
	{ 33322, "setvehiclelookattext" },
	{ 33323, "setvehicleteam" },
	{ 33324, "setneargoalnotifydist" },
	{ 33325, "setvehgoalpos" },
	{ 33326, "setgoalyaw" },
	{ 33327, "cleargoalyaw" },
	{ 33328, "settargetyaw" },
	{ 33329, "cleartargetyaw" },
	{ 33330, "helisetai" },
	{ 33331, "setturrettargetvec" },
	{ 33332, "setturrettargetent" },
	{ 33333, "clearturrettarget" },
	{ 33334, "canturretgettargetpoint" },
	{ 33335, "setlookatent" },
	{ 33336, "clearlookatent" },
	{ 33337, "setvehweapon" },
	{ 33338, "fireweapon" },
	{ 33339, "vehicleturretcontrolon" },
	{ 33340, "finishplayerdamage" },
	{ 33341, "suicide" },
	{ 33342, "closeingamemenu" },
	{ 33343, "iprintln" },
	{ 33344, "iprintlnbold" },
	{ 33345, "spawn" },
	{ 33346, "setentertime" },
	{ 33347, "cloneplayer" },
	{ 33348, "istalking" },
	{ 33349, "allowspectateteam" },
	{ 33350, "getguid" },
	{ 33351, "physicslaunchserver" },
	{ 33352, "physicslaunchserveritem" },
	{ 33353, "clonebrushmodeltoscriptmodel" },
	{ 33354, "scriptmodelplayanim" },
	{ 33355, "scriptmodelclearanim" },
	{ 33356, "vehicle_teleport" },
	{ 33357, "attachpath" },
	{ 33358, "getattachpos" },
	{ 33359, "startpath" },
	{ 33360, "setswitchnode" },
	{ 33361, "setwaitspeed" },
	{ 33362, "vehicle_finishdamage" },
	{ 33363, "setspeed" },
	{ 33364, "setspeedimmediate" },
	{ 33365, "vehicle_rotateyaw" },
	{ 33366, "getspeed" },
	{ 33367, "vehicle_getvelocity" },
	{ 33368, "getbodyvelocity" },
	{ 33369, "getsteering" },
	{ 33370, "getthrottle" },
	{ 33371, "turnengineoff" },
	{ 33372, "turnengineon" },
	{ 33373, "getgoalspeedmph" },
	{ 33374, "setacceleration" },
	{ 33375, "setdeceleration" },
	{ 33376, "resumespeed" },
	{ 33377, "setyawspeed" },
	{ 33378, "setyawspeedbyname" },
	{ 33379, "setmaxpitchroll" },
	{ 33380, "setairresistance" },
	{ 33381, "setturningability" },
	{ 33382, "getxuid" },
	{ 33383, "ishost" },
	{ 33384, "getspectatingplayer" },
	{ 33385, "predictstreampos" },
	{ 33386, "updatescores" },
	{ 33387, "updatedmscores" },
	{ 33388, "setrank" },
	{ 33389, "setcardtitle" },
	{ 33390, "weaponlocknoclearance" },
	{ 33391, "visionsyncwithplayer" },
	{ 33392, "showhudsplash" },
	{ 33393, "setperk" },
	{ 33394, "hasperk" },
	{ 33395, "clearperks" },
	{ 33396, "unsetperk" },
	{ 33397, "noclip" },
	{ 33398, "ufo" },
	{ 33399, "moveto" },
	{ 33400, "movex" },
	{ 33401, "movey" },
	{ 33402, "movez" },
	{ 33403, "movegravity" },
	{ 33404, "moveslide" },
	{ 33405, "stopmoveslide" },
	{ 33406, "rotateto" },
	{ 33407, "rotatepitch" },
	{ 33408, "rotateyaw" },
	{ 33409, "rotateroll" },
	{ 33410, "addpitch" },
	{ 33411, "addyaw" },
	{ 33412, "addroll" },
	{ 33413, "vibrate" },
	{ 33414, "rotatevelocity" },
	{ 33415, "solid" },
	{ 33416, "notsolid" },
	{ 33417, "setcandamage" },
	{ 33418, "setcanradiusdamage" },
	{ 33419, "physicslaunchclient" },
	{ 33420, "setcardicon" },
	{ 33421, "setcardnameplate" },
	{ 33422, "setcarddisplayslot" },
	{ 33423, "regweaponforfxremoval" },
	{ 33424, "laststandrevive" },
	{ 33425, "setspectatedefaults" },
	{ 33426, "getthirdpersoncrosshairoffset" },
	{ 33427, "disableweaponpickup" },
	{ 33428, "enableweaponpickup" },
	{ 33429, "issplitscreenplayer" },
	{ 33430, "getweaponslistoffhands" },
	{ 33431, "getweaponslistitems" },
	{ 33432, "getweaponslistexclusives" },
	{ 33433, "getweaponslist" },
	{ 33434, "canplayerplacesentry" },
	{ 33435, "canplayerplacetank" },
	{ 33436, "visionsetnakedforplayer" },
	{ 33437, "visionsetnightforplayer" },
	{ 33438, "visionsetmissilecamforplayer" },
	{ 33439, "visionsetthermalforplayer" },
	{ 33440, "visionsetpainforplayer" },
	{ 33441, "setblurforplayer" },
	{ 33442, "getplayerweaponmodel" },
	{ 33443, "getplayerknifemodel" },
	{ 33444, "updateplayermodelwithweapons" },
	{ 33445, "notifyonplayercommand" },
	{ 33446, "canmantle" },
	{ 33447, "forcemantle" },
	{ 33448, "ismantling" },
	{ 33449, "playfx" },
	{ 33450, "player_recoilscaleon" },
	{ 33451, "player_recoilscaleoff" },
	{ 33452, "weaponlockstart" },
	{ 33453, "weaponlockfinalize" },
	{ 33454, "weaponlockfree" },
	{ 33455, "weaponlocktargettooclose" },
	{ 33456, "issplitscreenplayerprimary" },
	{ 33457, "getviewmodel" },
	{ 33458, "fragbuttonpressed" },
	{ 33459, "secondaryoffhandbuttonpressed" },
	{ 33460, "getcurrentweaponclipammo" },
	{ 33461, "setvelocity" },
	{ 33462, "getplayerviewheight" },
	{ 33463, "getnormalizedmovement" },
	{ 33464, "setchannelvolumes" },
	{ 33465, "deactivatechannelvolumes" },
	{ 33466, "playlocalsound" },
	{ 33467, "stoplocalsound" },
	{ 33468, "setweaponammoclip" },
	{ 33469, "setweaponammostock" },
	{ 33470, "getweaponammoclip" },
	{ 33471, "getweaponammostock" },
	{ 33472, "anyammoforweaponmodes" },
	{ 33473, "setclientdvar" },
	{ 33474, "setclientdvars" },
	{ 33475, "allowads" },
	{ 33476, "allowjump" },
	{ 33477, "allowsprint" },
	{ 33478, "setspreadoverride" },
	{ 33479, "resetspreadoverride" },
	{ 33480, "setaimspreadmovementscale" },
	{ 33481, "setactionslot" },
	{ 33482, "setviewkickscale" },
	{ 33483, "getviewkickscale" },
	{ 33484, "getweaponslistall" },
	{ 33485, "getweaponslistprimaries" },
	{ 33486, "getnormalizedcameramovement" },
	{ 33487, "giveweapon" },
	{ 33488, "takeweapon" },
	{ 33489, "takeallweapons" },
	{ 33490, "getcurrentweapon" },
	{ 33491, "getcurrentprimaryweapon" },
	{ 33492, "getcurrentoffhand" },
	{ 33493, "hasweapon" },
	{ 33494, "switchtoweapon" },
	{ 33495, "switchtoweaponimmediate" },
	{ 33496, "switchtooffhand" },
	{ 33497, "setoffhandsecondaryclass" },
	{ 33498, "getoffhandsecondaryclass" },
	{ 33499, "beginlocationselection" },
	{ 33500, "endlocationselection" },
	{ 33501, "disableweapons" },
	{ 33502, "enableweapons" },
	{ 33503, "disableoffhandweapons" },
	{ 33504, "enableoffhandweapons" },
	{ 33505, "disableweaponswitch" },
	{ 33506, "enableweaponswitch" },
	{ 33507, "openpopupmenu" },
	{ 33508, "openpopupmenunomouse" },
	{ 33509, "closepopupmenu" },
	{ 33510, "openmenu" },
	{ 33511, "closemenu" },
	{ 33513, "freezecontrols" },
	{ 33514, "disableusability" },
	{ 33515, "enableusability" },
	{ 33516, "setwhizbyspreads" },
	{ 33517, "setwhizbyradii" },
	{ 33518, "setreverb" },
	{ 33519, "deactivatereverb" },
	{ 33520, "setvolmod" },
	{ 33521, "setchannelvolume" },
	{ 33522, "givestartammo" },
	{ 33523, "givemaxammo" },
	{ 33524, "getfractionstartammo" },
	{ 33525, "getfractionmaxammo" },
	{ 33526, "isdualwielding" },
	{ 33527, "isreloading" },
	{ 33528, "isswitchingweapon" },
	{ 33529, "setorigin" },
	{ 33530, "getvelocity" },
	{ 33531, "setplayerangles" },
	{ 33532, "getplayerangles" },
	{ 33533, "usebuttonpressed" },
	{ 33534, "attackbuttonpressed" },
	{ 33535, "adsbuttonpressed" },
	{ 33536, "meleebuttonpressed" },
	{ 33537, "playerads" },
	{ 33538, "isonground" },
	{ 33539, "isusingturret" },
	{ 33540, "setviewmodel" },
	{ 33541, "setoffhandprimaryclass" },
	{ 33542, "getoffhandprimaryclass" },
	{ 33543, "startac130" },
	{ 33544, "stopac130" },
	{ 33545, "error" },
	{ 33546, "setscriptmoverinkillcam" },
	{ 33547, "fadeovertime2" },
	{ 33548, "scaleovertime" },
};
std::string GetBuiltinMethodName(std::uint16_t id)
{
	if (builtinmethodmap.find(id) != builtinmethodmap.end())
		return builtinmethodmap[id];

	return "";
}


template <typename ... Args> void PrintInstruction(std::string format, Args ... args)
{
	size_t size = _snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	std::vector < char > buf;
	buf.resize(size);
	_snprintf(buf.data(), size, format.c_str(), args ...);
	std::string buffer = std::string(buf.data(), buf.data() + size - 1);

	printf(buffer.data());
}

std::uint32_t ParseCallOffset(std::unique_ptr<ByteBuffer>& script)
{
	std::vector < std::uint8_t > raw;

	// Resize vector to 4 bytes.
	raw.resize(4);
	std::fill(raw.begin(), raw.end(), 0);

	// Only do 3 iterations.
	for (int i = 0; i < 3; i++)
	{
		raw[i] = script->Read<std::uint8_t>();
	}

	// calculate real call offset.
	std::uint32_t offset = *reinterpret_cast<std::uint32_t*>(raw.data());
	offset = offset << 8;
	offset = offset >> 10;

	return offset;
}

std::uint32_t Parse_Opcode(std::unique_ptr<ByteBuffer>& script, std::unique_ptr<ByteBuffer>& stack, std::uint8_t opcode)
{
	std::uint32_t size = 1;

	PrintInstruction("\t");

	script->ParseOP();

	switch (opcode)
	{
	case OP_End:
		PrintInstruction("End");
		break;
	case OP_Return:
		PrintInstruction("Ret");
		break;
	case OP_vector:
		PrintInstruction("Vector");
		break;
	case OP_EvalArray:
		PrintInstruction("EvalArray");
		break;
	case OP_size:
		PrintInstruction("Size");
		break;
	case OP_AddArray:
		PrintInstruction("AddArray");
		break;
	case OP_EvalArrayRef:
		PrintInstruction("EvalArrayRef");
		break;
	case OP_JumpOnFalseExpr:
		PrintInstruction("JumpOnFalseExpr 0x%X", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_voidCodepos:
		PrintInstruction("VoidCodepos");
		break;
	case OP_PreScriptCall:
		PrintInstruction("PreScriptCall");
		break;
	case OP_notify:
		PrintInstruction("Notify");
		break;
	case OP_endon:
		PrintInstruction("Endon");
		break;
	case OP_GetByte:
		PrintInstruction("GetByte %i", script->Read<std::int8_t>());
		size += 1;
		break;
	case OP_GetNegByte:
		PrintInstruction("GetNegByte %u", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_GetUnsignedShort:
		PrintInstruction("GetUnsignedShort %i", script->Read<std::int16_t>());
		size += 2;
		break;
	case OP_GetNegUnsignedShort:
		PrintInstruction("GetNegUnsignedShort %u", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_GetInteger:
		PrintInstruction("GetInteger %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_GetBuiltinFunction:
		PrintInstruction("GetBuiltinFunction %i", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_GetBuiltinMethod:
		PrintInstruction("GetBuiltinMethod %i", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_GetFloat:
		PrintInstruction("GetFloat %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_GetUndefined:
		PrintInstruction("GetUndefined");
		break;
	case OP_GetZero:
		PrintInstruction("GetZero");
		break;
	case OP_GetString:
		PrintInstruction("GetString %s", stack->ReadString().data());
		script->Seek(2);
		size += 2;
		break;
	case OP_CreateLocalVariable:
		PrintInstruction("CreateLocalVariable %i", script->Read < std::uint8_t >());
		size += 1;
		break;
	case OP_RemoveLocalVariables:
		PrintInstruction("RemoveLocalVariables %i", script->Read < std::uint8_t >());
		size += 1;
		break;
	case OP_EvalLocalVariableCached0:
	case OP_EvalLocalVariableCached1:
	case OP_EvalLocalVariableCached2:
	case OP_EvalLocalVariableCached3:
	case OP_EvalLocalVariableCached4:
	case OP_EvalLocalVariableCached5:
		PrintInstruction("EvalLocalVariableCached%i", (opcode - OP_EvalLocalVariableCached0));
		break;
	case OP_EvalLocalVariableCached:
		PrintInstruction("EvalLocalVariableCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_EvalLocalArrayCached:
		PrintInstruction("EvalLocalArrayCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_EvalNewLocalArrayRefCached0:
		PrintInstruction("EvalNewLocalArrayRefCached0");
		break;
	case OP_EvalLocalArrayRefCached0:
		PrintInstruction("EvalLocalArrayRefCached0 %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_EvalLocalArrayRefCached:
		PrintInstruction("EvalLocalArrayRefCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ClearArray:
		PrintInstruction("ClearArray");
		break;
	case OP_EmptyArray:
		PrintInstruction("EmptyArray");
		break;
	case OP_ScriptLocalFunctionCall2:
		PrintInstruction("ScriptLocalFunctionCall2 %i", ParseCallOffset(script));
		size += 3;
		break;
	case OP_ScriptLocalFunctionCall:
		PrintInstruction("ScriptLocalFunctionCall %i", ParseCallOffset(script));
		size += 3;
		break;
	case OP_ScriptLocalMethodCall:
		PrintInstruction("ScriptLocalMethodCall %i", ParseCallOffset(script));
		size += 3;
		break;
	case OP_ScriptLocalThreadCall:
		PrintInstruction("ScriptLocalThreadCall %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ScriptLocalChildThreadCall:
		PrintInstruction("ScriptLocalChildThreadCall %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ScriptLocalMethodThreadCall:
		PrintInstruction("LocalMethodThreadCall %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ScriptLocalMethodChildThreadCall:
		PrintInstruction("ScriptLocalMethodChildThreadCall %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ScriptFarFunctionCall2:
		PrintInstruction("ScriptFarFunctionCall2 %i %s %s", ParseCallOffset(script), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 3;
		break;
	case OP_ScriptFarFunctionCall:
		PrintInstruction("ScriptFarFunctionCall %i %s %s", ParseCallOffset(script), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 3;
		break;
	case OP_ScriptFarMethodCall:
		PrintInstruction("ScriptFarMethodCall %i %s %s", ParseCallOffset(script), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 3;
		break;
	case OP_ScriptFarThreadCall:
		PrintInstruction("ScriptFarThreadCall %i %s %s", ParseCallOffset(script), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 4;
		break;
	case OP_ScriptFarChildThreadCall:
		PrintInstruction("ScriptFarChildThreadCall %i %s %s", ParseCallOffset(script), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 4;
		break;
	case OP_ScriptFarMethodThreadCall:
		PrintInstruction("ScriptFarMethodThreadCall %i %s %s", ParseCallOffset(script), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 4;
		break;
	case OP_ScriptFarMethodChildThreadCall:
		PrintInstruction("ScriptFarMethodChildThreadCall %i %s %s", ParseCallOffset(script), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 4;
		break;
	case OP_ScriptFunctionCallPointer:
		PrintInstruction("ScriptFunctionCallPointer");
		break;
	case OP_ScriptMethodCallPointer:
		PrintInstruction("ScriptMethodCallPointer");
		break;
	case OP_ScriptThreadCallPointer:
		PrintInstruction("ScriptThreadCallPointer %i", script->Read<std::uint8_t>());
		size += 1;
		break;
#ifdef IW5
	case 0x32:
		PrintInstruction("Error parsing opcode. Please reverse me!");
		break;
#endif
	case OP_ScriptMethodThreadCallPointer:
		PrintInstruction("ScriptMethodThreadCallPointer %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ScriptMethodChildThreadCallPointer:
		PrintInstruction("ScriptMethodChildThreadCallPointer %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_CallBuiltinPointer:
		PrintInstruction("CallBuiltinPointer 0x%X", script->Read<std::uint8_t>());
		size += 1;		// might be 4
		break;
	case OP_CallBuiltinMethodPointer:
		PrintInstruction("CallBuiltinMethodPointer %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_GetIString:
		PrintInstruction("GetIString %s", stack->ReadString().data());
		script->Seek(2);
		size += 2;
		break;
	case OP_GetVector:
		PrintInstruction("GetVector %f %f %f", script->Read<float>(), script->Read<float>(), script->Read<float>());
		size += 12;
		break;
	case OP_GetAnimObject:
		PrintInstruction("GetAnimObject %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_GetSelf:
		PrintInstruction("GetSelf");
		break;
	case OP_GetLevel:
		PrintInstruction("GetLevel");
		break;
	case OP_GetGame:
		PrintInstruction("GetGame");
		break;
	case OP_GetAnimation:
		PrintInstruction("GetAnimation %s %s", stack->ReadString().data(), stack->ReadString().data());
		break;
	case OP_GetGameRef:
		PrintInstruction("GameRef");
		break;
	case OP_inc:
		PrintInstruction("Inc");
		break;
	case OP_dec:
		PrintInstruction("Dec");
		break;
	case OP_bit_or:
		PrintInstruction("bit_or");
		break;
	case OP_bit_ex_or:
		PrintInstruction("bit_ex_or");
		break;
	case OP_bit_and:
		PrintInstruction("bit_and");
		break;
	case OP_equality:
		PrintInstruction("Equality");
		break;
	case OP_inequality:
		PrintInstruction("Inequality");
		break;
	case OP_less:
		PrintInstruction("Less");
		break;
	case OP_greater:
		PrintInstruction("Greater");
		break;
	case OP_JumpOnTrueExpr:
		PrintInstruction("JumpOnTrueExpr 0x%X", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_less_equal:
		PrintInstruction("LessEqual");
		break;
	case OP_jumpback:
		PrintInstruction("JumpBack 0x%X", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_waittill:
		PrintInstruction("WaitTill");
		break;

		// switch opcodes
	case OP_switch:
		PrintInstruction("Switch %i", script->Read<std::uint32_t>());
		size += 4;
		break;
	case OP_endswitch:

		// FIXME
		if (script->Read<std::uint16_t>())
		{
			// Rewind 2 bytes
			script->SeekNeg(sizeof(std::uint16_t));

			// Read it again.
			for (auto i = script->Read<std::uint16_t>(); i > 0; i--)
			{
				// Check switch label
				if (script->Read<std::uint32_t>() < 0x10000)
				{
					stack->ReadString();
				}

				// Increment read size by 4
				size += 4;

				// Read away the trash data
				script->Seek(3);

				// Increment read size by 3
				size += 3;
			}

			PrintInstruction("EndSwitch");
		}
		else
		{
			// Nothing seems to happen if the 2 bytes are 0 here.
			PrintInstruction("EndSwitch");
		}

		size += 2;
		break;

	case OP_JumpOnFalse:
		PrintInstruction("Jump<false> 0x%X", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_greater_equal:
		PrintInstruction("greater_equal");
		break;
	case OP_plus:
		PrintInstruction("Plus");
		break;
	case OP_jump:
		PrintInstruction("Jump 0x%X", script->Read<std::uint32_t>());
		size += 4;
		break;
	case OP_minus:
		PrintInstruction("Minus");
		break;

	case OP_multiply:
		PrintInstruction("Multiply");
		break;
	case OP_divide:
		PrintInstruction("Divide");
		break;

	case OP_JumpOnTrue:
		PrintInstruction("Jump<true> 0x%X", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_waittillmatch:
		PrintInstruction("WaitTillMatch 0x%X", script->Read<std::uint16_t>());
		size += 2;
		break;
	case OP_GetLocalFunction:
		PrintInstruction("GetLocalFunction %i %i", script->Read<std::uint8_t>(), script->Read<std::uint16_t>());
		size += 3;
		break;
	case OP_GetFarFunction:
		PrintInstruction("GetFarFunction %i %s %s", script->Read<std::uint8_t>(), stack->ReadOpaqueString().data(), stack->ReadOpaqueString().data());
		size += 3;
		break;
	case OP_GetSelfObject:
		PrintInstruction("GetSelfObject");
		break;

#define EVALFIELDVARIABLECASE(__OPCODE) \
	case OP_##__OPCODE: \
		if (script->Read<std::uint16_t>() > 33386) \
		{ \
			PrintInstruction(#__OPCODE " %s", stack->ReadOpaqueString().data()); \
		} \
		else \
		{ \
			script->SeekNeg(sizeof(std::uint16_t)); \
			PrintInstruction(#__OPCODE " %i", script->Read<std::uint16_t>()); \
		} \
		size += 2; \
		break;

		EVALFIELDVARIABLECASE(EvalLevelFieldVariable);
		EVALFIELDVARIABLECASE(EvalAnimFieldVariable);
		EVALFIELDVARIABLECASE(EvalSelfFieldVariable);
		EVALFIELDVARIABLECASE(EvalFieldVariable);
		EVALFIELDVARIABLECASE(EvalLevelFieldVariableRef);
		EVALFIELDVARIABLECASE(EvalAnimFieldVariableRef);
		EVALFIELDVARIABLECASE(EvalSelfFieldVariableRef);
		EVALFIELDVARIABLECASE(EvalFieldVariableRef);
		EVALFIELDVARIABLECASE(ClearFieldVariable);
		EVALFIELDVARIABLECASE(SetLevelFieldVariableField);
		EVALFIELDVARIABLECASE(SetAnimFieldVariableField);
		EVALFIELDVARIABLECASE(SetSelfFieldVariableField);

	
	case OP_SafeCreateVariableFieldCached:
		PrintInstruction("SafeCreateVariableFieldCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_SafeSetVariableFieldCached0:
		PrintInstruction("SafeSetVariableFieldCached0");
		break;
	case OP_SafeSetVariableFieldCached:
		PrintInstruction("SafeSetVariableFieldCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_SafeSetWaittillVariableFieldCached:
		PrintInstruction("SafeSetWaittillVariableFieldCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_GetAnimTree:
		PrintInstruction("GetAnimTree %s", stack->ReadString().data());
		script->Seek(1);
		size += 1;
		break;
	case OP_clearparams:
		PrintInstruction("clearparams");
		break;
	case OP_checkclearparams:
		PrintInstruction("checkclearparams");
		break;
	case OP_EvalLocalVariableRefCached0:
		PrintInstruction("EvalLocalVariableRefCached0");
		break;
	case OP_EvalNewLocalVariableRefCached0:
		PrintInstruction("EvalNewLocalVariableRefCached0");
		break;
	case OP_EvalLocalVariableRefCached:
		PrintInstruction("EvalLocalVariableRefCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_SetVariableField:
		PrintInstruction("SetVariableField");
		break;
	case OP_SetLocalVariableFieldCached0:
		PrintInstruction("SetLocalVariableFieldCached0");
		break;
	case OP_SetNewLocalVariableFieldCached0:
		PrintInstruction("SetNewLocalVariableFieldCached0 %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_SetLocalVariableFieldCached:
		PrintInstruction("SetLocalVariableFieldCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ClearLocalVariableFieldCached:
		PrintInstruction("ClearLocalVariableFieldCached %i", script->Read<std::uint8_t>());
		size += 1;
		break;
	case OP_ClearLocalVariableFieldCached0:
		PrintInstruction("ClearLocalVariableFieldCached0");
		break;
	case OP_CallBuiltin0:
	case OP_CallBuiltin1:
	case OP_CallBuiltin2:
	case OP_CallBuiltin3:
	case OP_CallBuiltin4:
	case OP_CallBuiltin5:
		PrintInstruction("Call<%i> %s", (opcode - OP_CallBuiltin0), GetBuiltinFuncName(script->Read<std::uint16_t>()).data());
		size += 2;
		break;
	case OP_CallBuiltin:
		PrintInstruction("Call %i", ParseCallOffset(script));
		size += 3;
		break;
	case OP_CallBuiltinMethod0:
	case OP_CallBuiltinMethod1:
	case OP_CallBuiltinMethod2:
	case OP_CallBuiltinMethod3:
	case OP_CallBuiltinMethod4:
	case OP_CallBuiltinMethod5:
		PrintInstruction("Method<%i> %s", (opcode - OP_CallBuiltinMethod0), GetBuiltinMethodName(script->Read<std::uint16_t>()).data());
		size += 2;
		break;
	case OP_CallBuiltinMethod:
		PrintInstruction("Method %i", ParseCallOffset(script));
		size += 3;
		break;
	case OP_wait:
		PrintInstruction("Wait");
		break;
	case OP_DecTop:
		PrintInstruction("DecTop");
		break;
	case OP_CastFieldObject:
		PrintInstruction("CastFieldObject");
		break;
	case OP_EvalLocalVariableObjectCached:
		PrintInstruction("EvalLocalVariableObjectCached %i", script->Read < std::uint8_t >());
		size += 1;
		break;
	case OP_CastBool:
		PrintInstruction("CastBool");
		break;
	case OP_BoolNot:
		PrintInstruction("BoolNot");
		break;
	case OP_BoolComplement:
		PrintInstruction("BoolComplement");
		break;

	default:
		PrintInstruction("unhandled opcode 0x%X", opcode);
		break;
	}

	PrintInstruction("\t\t#%s\n", script->GetConsumedBytes().data());

	return size;
}

void Assembler_Init();
void Assemble(std::string scriptname);

void Disassemble(std::string file)
{
	if (file.find(".stack") != std::string::npos)
	{
		printf("Cannot disassemble stack files\n");
		return;
	}

	const auto ext = std::string(".cgsc");
	const auto extpos = file.find(ext);
	if (extpos != std::string::npos)
	{
		file.replace(extpos, ext.length(), "");
	}

	// open files
	auto script = std::make_unique<ByteBuffer>(file + ".cgsc");
	auto stack = std::make_unique<ByteBuffer>(file + ".cgsc.stack");

	// Read begin opcode of the scriptfile
	auto op = script->Read<std::uint8_t>();

	while (stack->is_avail() && script->is_avail())
	{
		// read current function
		auto funcsize = stack->Read<std::uint32_t>();
		auto funcid = stack->Read<std::uint16_t>();

		PrintInstruction("fn_%i:\n", funcid);
		while (funcsize > 0)
		{
			op = script->Read<std::uint8_t>();

			// Parse opcode
			funcsize -= Parse_Opcode(script, stack, op);
		}

		PrintInstruction("\n");
	}
}

int main(int argc, char** argv)
{
	Assembler_Init();

	std::string file = argv[argc - 1];
	std::string mode;
	auto toConsole = false;

	if (argc >= 3)
	{
		for (int i = 1; i < argc - 1; i++)
		{
			std::string arg = argv[i];

			if (arg == "-stdout")
			{
				toConsole = true;
			}
			else if (arg == "-disasm" || arg == "-asm")
			{
				mode = arg;
			}
			else
			{
				printf("Unknown flag \"%s\".\n", argv[i]);
			}
		}
	}
	else
	{
		printf("usage: ScriptfileDisasm.exe <options> <file>\n");
		return 0;
	}

	if (!toConsole)
	{
		freopen(std::string(file + ".gscasm").c_str(), "w", stdout);
	}
	else if (mode == "-asm")
	{
		printf("-stdout can only be used with -disasm\n");
		return 1;
	}

	if (mode == "-asm")
	{
		Assemble(file);
	}
	else
	{
		Disassemble(file);
	}

    return 0;
}

