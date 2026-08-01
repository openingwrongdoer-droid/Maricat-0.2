#include <iostream>

#include <fstream>

#include <variant>
#include <vector>
#include <map>

#include <filesystem>

#include <ctime>
#include <random>
#include <cmath>

#include <nlohmann/json.hpp>

bool frameAdvance() /*{
	return 0;
}*/;

short getKeyData(std::string name)/* {
	return 0;
}*/;

bool mouseCollisionCheck(int x, int y, int i) /*{
	return 0;
}*/;

bool spriteCollisionCheck(int i, int j) /*{
	return 0;
}*/;

bool edgeCollisionCheck(int i) /*{
	return 0;
}*/;

namespace mariEng {
	bool greetings = true;

	using namespace std;

	using json = nlohmann::json;

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

	using doubleType = variant<float, string>;

	struct Block {
		json fields;
		json inputs;
		string next;
		string opcode;
	};

	struct Costume {
		string md5ext;
		string name;
		int rotationCenterX;
		int rotationCenterY;
		Texture texture;
	};

	struct Sprite {
		string name;
		map<string, Block> blocks;
		vector<string> event_wfc;
		map<string, string> varNames;
		map<string, doubleType> variables;
		map<string, vector<string>> lists;
		float x;
		float y;
		//-180 - 180
		float direction;
		string rotationStyle;
		vector<Costume> costumes;
		int currentCostume;
		int size;
		struct {
			float color = 0;
			float fisheye = 0;
			float whirl = 0;
			float pixelate = 0;
			float mosaic = 0;
			float brightness = 0;
			float ghost = 0;
		} effects;
		bool visible;
		int layerOrder;
		int volume;
		
		bool draggable;
		
		vector<string> nestStack;
		
		float timer = 0;
		
		float wait_duration;
		time_t wait_startingTime;
		
		float glide_startingX;
		float glide_startingY;
		float glide_endX;
		float glide_endY;
		float glide_secs;
		time_t glide_startingTime;
	};

	vector<Sprite> sprites;

	string currBlock;

	struct {
		int x = 0;
		int y = 0;
		short action = 0;
	} mouse;

	time_t programStart;

	struct {
		string answer = "";
		int loudness = 0;
		float timer;
		string username = "";
		string online = "false";
	} globalSens;
	
	doubleType getVariable (int spr, string name) {
		return sprites[0].variables.find(name) != sprites[0].variables.end() ? sprites[0].variables[name] : sprites[spr].variables[name];
	}

	void setVariable (int spr, string name, doubleType value) {
		if (sprites[0].variables.find(name) != sprites[0].variables.end()) {
			sprites[0].variables[name] = value;
		} else {
			sprites[spr].variables[name] = value;
		}
	}

	vector<string>& getList (int spr, string name) {
		return sprites[0].lists.find(name) != sprites[spr].lists.end() ? sprites[0].lists[name] : sprites[spr].lists[name];
	}

	int spriteIndexFromName(string name) {
		for (int i = 0; i < sprites.size(); i++) {
			if (sprites[i].name == name) return i;
		}
		return -1;
	}

	int costumeIndexFromName(int spr, string name) {
		for (int i = 0; i < sprites[spr].costumes.size(); i++) {
			if (sprites[spr].costumes[i].name == name) return i;
		}
		return -1;
	}

	doubleType handleBlockInputs(int spr, string thisName, string key);

	string accToStr (float s) {
		if (s != floor(s)) return to_string(s);
		string s1 = to_string(s);
		s1.erase(s1.find('.'), s1.size());
		return s1;
	}

	string dToStr(doubleType d) {
		return holds_alternative<string>(d) ? get<string>(d) : accToStr(get<float>(d));
	}
	float dToNum(doubleType res) {
		return holds_alternative<float>(res) ? get<float>(res) : (get<string>(res) == "true" ? 1 : (get<string>(res) == "false" ? 0 : stof(get<string>(res))));
	}

	float accStof (string s) {
		return s == "" ? 0 : stof(s);
	}
	
	bool executeScript (int spr);

	doubleType handleInputsLogic(int spr, string thisName) {
		Block thisBlock = sprites[spr].blocks[thisName];

		if (thisBlock.opcode[0] == 'm') {
			if (thisBlock.opcode == "motion_xposition") {
				return (float)sprites[spr].x;
			} else if (thisBlock.opcode == "motion_yposition") {
				return (float)sprites[spr].y;
			} else if (thisBlock.opcode == "motion_direction") {
				return (float)sprites[spr].direction;
			}
		} else if (thisBlock.opcode[0] == 'l') {
			if (thisBlock.opcode == "looks_costumenumbername") {
				if (thisBlock.fields["NUMBER_NAME"][0] == "number") {
					return (float)sprites[spr].currentCostume + 1;
				} else {
					return sprites[spr].costumes[sprites[spr].currentCostume].name;
				}
			} else if (thisBlock.opcode == "looks_backdropnumbername") {
				if (thisBlock.fields["NUMBER_NAME"][0] == "number") {
					return (float)sprites[0].currentCostume + 1;
				} else {
					return sprites[0].costumes[sprites[0].currentCostume].name;
				}
			} else if (thisBlock.opcode == "looks_size") {
				return (float)sprites[spr].size;
			}
		} else if (thisBlock.opcode[0] == 's' && thisBlock.opcode[1] == 'o') {
			if (thisBlock.opcode == "sound_volume") {
				return (float)sprites[spr].volume;
			}
		} else if (thisBlock.opcode[0] == 's' && thisBlock.opcode[1] == 'e') {
			if (thisBlock.opcode == "sensing_touchingobject") {
				Block touchingobject_menu = sprites[spr].blocks[thisBlock.inputs["TOUCHINGOBJECTMENU"][1]];
				if (touchingobject_menu.opcode == "sensing_touchingobjectmenu") {
					if (touchingobject_menu.fields["TOUCHINGOBJECTMENU"][0] == "_mouse_") {
						return mouseCollisionCheck(mouse.x, mouse.y, spr) ? "true" : "false";
					} else if (touchingobject_menu.fields["TOUCHINGOBJECTMENU"][0] == "_edge_") {
						return edgeCollisionCheck(spr) ? "true" : "false";
					} else {
						return spriteCollisionCheck(spriteIndexFromName(touchingobject_menu.fields["TOUCHINGOBJECTMENU"][0]), spr) ? "true" : "false";
					}
				} else return "false";
				
			} else if (thisBlock.opcode == "sensing_distanceto") {
				Block distancetomenu = sprites[spr].blocks[thisBlock.inputs["DISTANCETOMENU"][1]];
				if (distancetomenu.opcode == "sensing_distancetomenu") {
					if (distancetomenu.fields["DISTANCETOMENU"][0] == "_mouse_") {
						return (float)sqrt(pow(mouse.x - sprites[spr].x, 2) + pow(mouse.y + sprites[spr].y, 2));
					} else {
						int index = spriteIndexFromName(distancetomenu.fields["DISTANCETOMENU"][0]);
						return (float)sqrt(pow(sprites[index].x - sprites[spr].x, 2) + pow(sprites[index].y - sprites[spr].y, 2));
					}
				} else {
					return 10000.0f;
				}
			} else if (thisBlock.opcode == "sensing_answer") {
				return globalSens.answer;
			} else if (thisBlock.opcode == "sensing_keypressed") {
				Block keyoptions = sprites[spr].blocks[thisBlock.inputs["KEY_OPTION"][1]];
				if (keyoptions.opcode != "sensing_keyoptions") return "false";
				string KEY_OPTION = keyoptions.fields["KEY_OPTION"][0];
				return (getKeyData(KEY_OPTION) & 4) ? "true" : "false";
			} else if (thisBlock.opcode == "sensing_mousedown") {
				return (mouse.action & 2) ? "true" : "false";
			} else if (thisBlock.opcode == "sensing_mousex") {
				return (float)mouse.x;
			} else if (thisBlock.opcode == "sensing_mousey") {
				return (float)mouse.y;
			} else if (thisBlock.opcode == "sensing_loudness") {
				return (float)globalSens.loudness;
			} else if (thisBlock.opcode == "sensing_timer") {
				return globalSens.timer;
			} else if (thisBlock.opcode == "sensing_of") {
				Block ofobjectmenu = sprites[spr].blocks[thisBlock.inputs["OBJECT"][1]];

				if (ofobjectmenu.opcode == "sensing_of_object_menu") {
					int index;
					if (ofobjectmenu.fields["OBJECT"][0] == "_stage_") index = 0;
					else index = spriteIndexFromName(ofobjectmenu.fields["OBJECT"][0]);
					
					if (thisBlock.fields["PROPERTY"][0] == "backdrop #" || thisBlock.fields["PROPERTY"][0] == "costume #") {
						return (float)sprites[index].currentCostume + 1;
					} else if (thisBlock.fields["PROPERTY"][0] == "backdrop name" || thisBlock.fields["PROPERTY"] == "costume name") {
						return sprites[index].costumes[sprites[index].currentCostume].name;
					} else if (thisBlock.fields["PROPERTY"][0] == "x position") {
						return (float)sprites[index].x;
					} else if (thisBlock.fields["PROPERTY"][0] == "y position") {
						return (float)sprites[index].y;
					} else if (thisBlock.fields["PROPERTY"][0] == "direction") {
						return (float)sprites[index].direction;
					} else if (thisBlock.fields["PROPERTY"][0] == "size") {
						return (float)sprites[index].size;
					} else if (thisBlock.fields["PROPERTY"][0] == "volume") {
						return (float)sprites[index].volume;
					} else {
						return sprites[index].variables[sprites[index].varNames[thisBlock.fields["PROPERTY"][0]]];
					}
				} else {
					return 0.0f;
				}
			} else if (thisBlock.opcode == "sensing_current") {
				time_t timestamp = time(&timestamp);
				struct tm datetime = *localtime(&timestamp);
				if (thisBlock.fields["CURRENTMENU"][0] == "YEAR") {
					return (float)datetime.tm_year + 1900;
				} else if (thisBlock.fields["CURRENTMENU"][0] == "MONTH") {
					return (float)datetime.tm_mon + 1;
				} else if (thisBlock.fields["CURRENTMENU"][0] == "DATE") {
					return (float)datetime.tm_mday;
				} else if (thisBlock.fields["CURRENTMENU"][0] == "DAYOFWEEK") {
					return (float)datetime.tm_wday + 1;
				} else if (thisBlock.fields["CURRENTMENU"][0] == "HOUR") {
					return (float)datetime.tm_hour;
				} else if (thisBlock.fields["CURRENTMENU"][0] == "MINUTE") {
					return (float)datetime.tm_min;
				} else if (thisBlock.fields["CURRENTMENU"][0] == "SECOND") {
					return (float)datetime.tm_sec;
				}
				return 0.0f;
			} else if (thisBlock.opcode == "sensing_dayssince2000") {
				struct tm a = {0, 0, 0, 1, 0, 100};
				time_t timestamp = time(&timestamp);
				struct tm b = *localtime(&timestamp);
				time_t x = mktime(&a);
				time_t y = mktime(&b);
				double diff = difftime(y, x) / (60 * 60 * 24);
				return (float)diff;
			} else if (thisBlock.opcode == "sensing_username") {
				return globalSens.username;
			} else if (thisBlock.opcode == "sensing_online") {
				return globalSens.online;
			}
		} else if (thisBlock.opcode[0] == 'o') {
			float NUM1 = 0;
			float NUM2 = 0;
			int FROM = 0;
			int TO = 0;
			float OPERAND1;
			float OPERAND2;
			bool OPERAND;
			string STRING1;
			string STRING2;
			int LETTER = 0;
			string STRING;
			float NUM = 0;
			
			if (thisBlock.inputs["NUM1"] != nullptr) {
				doubleType res = handleBlockInputs(spr, thisName, "NUM1");
				NUM1 = dToNum(res);
				res = handleBlockInputs(spr, thisName, "NUM2");
				NUM2 = dToNum(res);
			} else if (thisBlock.inputs["FROM"] != nullptr) {
				doubleType res = handleBlockInputs(spr, thisName, "FROM");
				FROM = dToNum(res);
				res = handleBlockInputs(spr, thisName, "TO");
				TO = dToNum(res);
			} else if (thisBlock.inputs["OPERAND1"] != nullptr) {
				doubleType res = handleBlockInputs(spr, thisName, "OPERAND1");
				OPERAND1 = dToNum(res);
				res = handleBlockInputs(spr, thisName, "OPERAND2");
				OPERAND2 = dToNum(res);
			} else if (thisBlock.inputs["STRING1"] != nullptr) {
				doubleType res = handleBlockInputs(spr, thisName, "STRING1");
				STRING1 = dToStr(res);
				res = handleBlockInputs(spr, thisName, "STRING2");
				STRING2 = dToStr(res);
			} else if (thisBlock.inputs["OPERAND"] != nullptr) {
				doubleType res = handleBlockInputs(spr, thisName, "OPERAND");
				OPERAND = dToNum(res);
			} else if (thisBlock.inputs["STRING"] != nullptr) {
				doubleType res = handleBlockInputs(spr, thisName, "STRING");
				STRING = dToStr(res);
				res = handleBlockInputs(spr, thisName, "LETTER");
				LETTER = dToNum(res);
			}  else if (thisBlock.inputs["NUM"] != nullptr) {
				doubleType res = handleBlockInputs(spr, thisName, "NUM");
				NUM = dToNum(res);
			}

			if (thisBlock.opcode == "operator_add") {
				return NUM1 + NUM2;
			} else if (thisBlock.opcode == "operator_subtract") {
				return NUM1 - NUM2;
			} else if (thisBlock.opcode == "operator_multiply") {
				return NUM1 * NUM2;
			} else if (thisBlock.opcode == "operator_divide") {
				return NUM1 / NUM2;
			} else if (thisBlock.opcode == "operator_random") {
				srand(time(0));
				return float(rand() % (TO - FROM + 1) + FROM);
			} else if (thisBlock.opcode == "operator_gt") {
				return OPERAND1 > OPERAND2 ? "true" : "false";
			} else if (thisBlock.opcode == "operator_lt") {
				return OPERAND1 < OPERAND2 ? "true" : "false";
			} else if (thisBlock.opcode == "operator_equals") {
				return OPERAND1 == OPERAND2 ? "true" : "false";
			} else if (thisBlock.opcode == "operator_and") {
				return bool((int)OPERAND1 & (int)OPERAND2) ? "true" : "false";
			} else if (thisBlock.opcode == "operator_or") {
				return bool((int)OPERAND1 | (int)OPERAND2) ? "true" : "false";
			} else if (thisBlock.opcode == "operator_not") {
				return bool((int)OPERAND ^ 1) ? "true" : "false";
			} else if (thisBlock.opcode == "operator_join") {
				return STRING1 + STRING2;
			} else if (thisBlock.opcode == "operator_letter_of") {
				return STRING[LETTER] + "";
			} else if (thisBlock.opcode == "operator_length") {
				return float(STRING.size());
			} else if (thisBlock.opcode == "operator_contains") {
				return STRING1.find(STRING2) != string::npos ? "true" : "false";
			} else if (thisBlock.opcode == "operator_mod") {
				return float((int)NUM1 % (int)NUM2);
			} else if (thisBlock.opcode == "operator_round") {
				return round(NUM);
			} else if (thisBlock.opcode == "operator_mathop") {
				if (thisBlock.fields["OPERATOR"][0] == "abs") {
					return abs(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "floor") {
					return floor(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "ceiling") {
					return ceil(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "sqrt") {
					return sqrt(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "sin") {
					return sin(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "cos") {
					return cos(NUM);
				}  else if (thisBlock.fields["OPERATOR"][0] == "tan") {
					return tan(NUM);
				}  else if (thisBlock.fields["OPERATOR"][0] == "asin") {
					return asin(NUM);
				}  else if (thisBlock.fields["OPERATOR"][0] == "acos") {
					return acos(NUM);
				}  else if (thisBlock.fields["OPERATOR"][0] == "atan") {
					return atan(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "ln") {
					return log(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "log") {
					return log10(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "e ^") {
					return exp(NUM);
				} else if (thisBlock.fields["OPERATOR"][0] == "10 ^") {
					return (float)pow(10, NUM);
				}
			}
		} else if (thisBlock.opcode[0] == 'd') {
			if (thisBlock.opcode == "data_itemoflist") {
				int INDEX = stof(dToStr(handleBlockInputs(spr, thisName, "INDEX")));
				string LIST = thisBlock.fields["LIST"][1];
				return getList(spr, LIST)[INDEX - 1];
			} else if (thisBlock.opcode == "data_itemnumoflist") {
				string ITEM = dToStr(handleBlockInputs(spr, thisName, "ITEM"));
				string LIST = thisBlock.fields["LIST"][1];
				for (int i = 0; i < getList(spr, LIST).size(); i++) {
					if (getList(spr, LIST)[i] == ITEM) return float(i + 1);
				}
				return 0.0f;
			} else if (thisBlock.opcode == "data_lengthoflist") {
				string LIST = thisBlock.fields["LIST"][1];
				return (float)getList(spr, LIST).size();
			} else if (thisBlock.opcode == "data_listcontainsitem") {
				string ITEM = dToStr(handleBlockInputs(spr, thisName, "ITEM"));
				string LIST = thisBlock.fields["LIST"][1];
				for (auto i : getList(spr, LIST)) {
					if (i == ITEM) return "true";
				}
				return "false";
			}
		}
		return 0.0f;
	}

	doubleType handleBlockInputs(int spr, string thisName, string key) {
		Block thisBlock = sprites[spr].blocks[thisName];
		if ((thisBlock.inputs[key][0] == 3 || thisBlock.inputs[key][0] == 2) && thisBlock.inputs[key][1].is_array() == false) {
			doubleType res = handleInputsLogic(spr, thisBlock.inputs[key][1]);
			string ret = holds_alternative<float>(res) ? accToStr(get<float>(res)) : (holds_alternative<string>(res) ? get<string>(res) : "0");
			return ret;
		} else if (thisBlock.inputs[key][0] == 1) {
			json res;
			if (thisBlock.inputs[key][1].is_array()) {
				res = thisBlock.inputs[key][1][1];
			} else {
				res = thisBlock.inputs[key][1];
			}
			return res == "" ? "0" : (res.type() == json::value_t::number_unsigned ? to_string(res.get<unsigned int>()) : res.get<string>());
		} else if (thisBlock.inputs[key][1][0] == 12) {
			return getVariable(spr, thisBlock.inputs[key][1][2]);
		} else if (thisBlock.inputs[key][1][0] == 13) {
			string ret = "";
			for (string i : getList(spr, thisBlock.inputs[key][1][2])) {
				ret += " " + i;
			}
			ret.erase(0, 1);
			return ret;
		}
		return " ";
	}

	void handleBlocks (int spr, string thisName) {
		Block thisBlock = sprites[spr].blocks[thisName];
		if (thisBlock.opcode[0] == 'm') {
			if (thisBlock.opcode == "motion_movesteps") {
				float STEPS = dToNum(handleBlockInputs(spr, thisName, "STEPS"));
				sprites[spr].x -= STEPS * sin((sprites[spr].direction + 180) / 360 * 6.2831f);
				sprites[spr].y -= STEPS * cos((sprites[spr].direction + 180) / 360 * 6.2831f);
			} else if (thisBlock.opcode == "motion_turnright") {
				float DEGREES = dToNum(handleBlockInputs(spr, thisName, "DEGREES"));
				float rot = sprites[spr].direction + DEGREES;
				if (rot > 180) rot = -180 + fmod(rot, 180);
				if (rot < -180) rot = 180 - fmod(rot, 180);
				sprites[spr].direction = rot;
			} else if (thisBlock.opcode == "motion_turnleft") {
				float DEGREES = dToNum(handleBlockInputs(spr, thisName, "DEGREES"));
				float rot = sprites[spr].direction - DEGREES;
				if (rot > 180) rot = -180 + fmod(rot, 180);
				if (rot < -180) rot = 180 - fmod(rot, 180);
				sprites[spr].direction = rot;
			} else if (thisBlock.opcode == "motion_goto") {
				Block goto_menu = sprites[spr].blocks[thisBlock.inputs["TO"][1]];
				if (goto_menu.opcode == "motion_goto_menu") {
					if (goto_menu.fields["TO"][0] == "_random_") {
						srand(time(0));
						sprites[spr].x = rand() % 480 - 240;
						sprites[spr].y = rand() % 360 - 180;
					} else if (goto_menu.fields["TO"][0] == "_mouse_") {
						sprites[spr].x = mouse.x;
						sprites[spr].y = mouse.y;
					} else {
						int index = spriteIndexFromName(goto_menu.fields["TO"][0]);
						sprites[spr].x = sprites[index].x;
						sprites[spr].y = sprites[index].y;
					}
				}
			} else if (thisBlock.opcode == "motion_gotoxy") {
				float X = dToNum(handleBlockInputs(spr, thisName, "X"));
				float Y = dToNum(handleBlockInputs(spr, thisName, "Y"));
				sprites[spr].x = X;
				sprites[spr].y = Y;
			} else if (thisBlock.opcode == "motion_glidesecstoxy") {
				if (sprites[spr].timer == 0) {
					float X = dToNum(handleBlockInputs(spr, thisName, "X"));
					float Y = dToNum(handleBlockInputs(spr, thisName, "Y"));
					float SECS = dToNum(handleBlockInputs(spr, thisName, "SECS"));
					sprites[spr].glide_startingX = sprites[spr].x;
					sprites[spr].glide_startingY = sprites[spr].y;
					sprites[spr].glide_endX = X;
					sprites[spr].glide_endY = Y;
					sprites[spr].glide_secs = SECS;
					sprites[spr].glide_startingTime = clock();
					sprites[spr].timer = 1;
				}
				float deltaTime = double(clock() - sprites[spr].glide_startingTime) / CLOCKS_PER_SEC / sprites[spr].glide_secs;
				if (deltaTime >= 1) {
					deltaTime = 1;
					sprites[spr].timer = 0;
				}
				sprites[spr].x = (1 - deltaTime) * sprites[spr].glide_startingX + deltaTime * sprites[spr].glide_endX;
				sprites[spr].y = (1 - deltaTime) * sprites[spr].glide_startingY + deltaTime * sprites[spr].glide_endY;
			} else if (thisBlock.opcode == "motion_glideto") {
				if (sprites[spr].timer == 0) {
					float SECS = dToNum(handleBlockInputs(spr, thisName, "SECS"));
					sprites[spr].glide_startingX = sprites[spr].x;
					sprites[spr].glide_startingY = sprites[spr].y;
					sprites[spr].glide_secs = SECS;
					sprites[spr].glide_startingTime = clock();
					sprites[spr].timer = 1;
					
					Block glideto_menu = sprites[spr].blocks[thisBlock.inputs["TO"][1]];
					cout << glideto_menu.fields["TO"][0];
					if (glideto_menu.opcode == "motion_glideto_menu") {
						//TODO put this in goto too
						if (glideto_menu.fields["TO"][0] == "_random_") {
							srand(time(0));
							sprites[spr].glide_endX = rand() % 480 - sprites[spr].costumes[sprites[spr].currentCostume].rotationCenterX;
							sprites[spr].glide_endY = rand() % 360 - sprites[spr].costumes[sprites[spr].currentCostume].rotationCenterY;
						} else if (glideto_menu.fields["TO"][0] == "_mouse_") {
							sprites[spr].glide_endX = mouse.x - sprites[spr].costumes[sprites[spr].currentCostume].rotationCenterX;
							sprites[spr].glide_endY = mouse.y - sprites[spr].costumes[sprites[spr].currentCostume].rotationCenterY;
						} else {
							int index = spriteIndexFromName(glideto_menu.fields["TO"][0]);
							sprites[spr].glide_endX = sprites[index].x - sprites[spr].costumes[sprites[spr].currentCostume].rotationCenterX;
							sprites[spr].glide_endY = sprites[index].y - sprites[spr].costumes[sprites[spr].currentCostume].rotationCenterY;
						}
					}
				}
				float deltaTime = double(clock() - sprites[spr].glide_startingTime) / CLOCKS_PER_SEC / sprites[spr].glide_secs;
				std::cout << sprites[spr].glide_endX << ' ' << sprites[spr].glide_endY << " - ";
				if (deltaTime >= 1) {
					deltaTime = 1;
					sprites[spr].timer = 0;
				}
				sprites[spr].x = (1 - deltaTime) * sprites[spr].glide_startingX + deltaTime * sprites[spr].glide_endX;
				sprites[spr].y = (1 - deltaTime) * sprites[spr].glide_startingY + deltaTime * sprites[spr].glide_endY;
			} else if (thisBlock.opcode == "motion_pointindirection") {
				float DIRECTION = dToNum(handleBlockInputs(spr, thisName, "DIRECTION"));
				sprites[spr].direction = DIRECTION;
			} else if (thisBlock.opcode == "motion_pointtowards") {
				Block goto_menu = sprites[spr].blocks[thisBlock.inputs["TOWARDS"][1]];
				if (goto_menu.opcode == "motion_pointtowards_menu") {
					if (goto_menu.fields["TOWARDS"][0] == "_mouse_") {
						sprites[spr].direction = (atan2(mouse.y - sprites[spr].y, mouse.x - sprites[spr].x) + 3.1415f) / 6.2831f * 360.0f;
					} else {
						int index = spriteIndexFromName(goto_menu.fields["TOWARDS"][0]);
						sprites[spr].direction = (atan2(sprites[index].y - sprites[spr].y, sprites[index].x - sprites[spr].x) + 3.1415f) / 6.2831f * 360.0f;
					}
				}
			} else if (thisBlock.opcode == "motion_changexby") {
				float DX = dToNum(handleBlockInputs(spr, thisName, "DX"));
				sprites[spr].x += DX;
			} else if (thisBlock.opcode == "motion_setx") {
				float X = dToNum(handleBlockInputs(spr, thisName, "X"));
				sprites[spr].x = X;
			} else if (thisBlock.opcode == "motion_changeyby") {
				float DY = dToNum(handleBlockInputs(spr, thisName, "DY"));
				sprites[spr].y += DY;
			} else if (thisBlock.opcode == "motion_sety") {
				float Y = dToNum(handleBlockInputs(spr, thisName, "Y"));
				sprites[spr].y = Y;
			} else if (thisBlock.opcode == "motion_setrotationstyle") {
				string STYLE = thisBlock.fields["STYLE"][0];
				sprites[spr].rotationStyle = STYLE;
			}
		} else if (thisBlock.opcode[0] == 'l') {
			if (thisBlock.opcode == "looks_say") {
				string MESSAGE = dToStr(handleBlockInputs(spr, thisName, "MESSAGE"));
				if (MESSAGE != "") cout << "\n" << sprites[spr].name << " says:" << MESSAGE;
			} else if (thisBlock.opcode == "looks_think") {
				string MESSAGE = dToStr(handleBlockInputs(spr, thisName, "MESSAGE"));
				if (MESSAGE != "") cout << "\n" << sprites[spr].name << " thinks:" << MESSAGE;
			} else if (thisBlock.opcode == "looks_switchcostumeto") {
				Block COSTUME = sprites[spr].blocks[thisBlock.inputs["COSTUME"][1]];
				int index;
				if (COSTUME.opcode == "looks_costume") {
					index = costumeIndexFromName(spr, COSTUME.fields["COSTUME"][0]);
				} else {
					index = costumeIndexFromName(spr, dToStr(handleBlockInputs(spr, thisName, "COSTUME")));
				}
				if (index != -1) {
					sprites[spr].currentCostume = index;
				}
			} else if (thisBlock.opcode == "looks_nextcostume") {
				if (sprites[spr].currentCostume + 1 < sprites[spr].costumes.size()) sprites[spr].currentCostume++;
			} else if (thisBlock.opcode == "looks_switchbackdropto") {
				Block COSTUME = sprites[spr].blocks[thisBlock.inputs["BACKDROP"][1]];
				int index;
				if (COSTUME.opcode == "looks_costume") {
					index = costumeIndexFromName(0, COSTUME.fields["BACKDROP"][0]);
				} else {
					index = costumeIndexFromName(0, dToStr(handleBlockInputs(spr, thisName, "BACKDROP")));
				}
				if (index != -1) {
					sprites[0].currentCostume = index;
				}
			} else if (thisBlock.opcode == "looks_nextbackdrop") {
				if (sprites[0].currentCostume + 1 < sprites[0].costumes.size()) sprites[0].currentCostume++;
			} else if (thisBlock.opcode == "looks_changesizeby") {
				sprites[spr].size += dToNum(handleBlockInputs(spr, thisName, "CHANGE"));
			} else if (thisBlock.opcode == "looks_setsizeto") {
				sprites[spr].size = dToNum(handleBlockInputs(spr, thisName, "SIZE"));
			} else if (thisBlock.opcode == "looks_changeeffectby") {
				string EFFECT = thisBlock.fields["EFFECT"][0];
				float CHANGE = dToNum(handleBlockInputs(spr, thisName, "CHANGE"));
				if (EFFECT == "COLOR") {
					sprites[spr].effects.color += CHANGE;
				} else if (EFFECT == "COLOR") {
					sprites[spr].effects.color += CHANGE;
				} else if (EFFECT == "FISHEYE") {
					sprites[spr].effects.fisheye += CHANGE;
				} else if (EFFECT == "WHIRL") {
					sprites[spr].effects.whirl += CHANGE;
				} else if (EFFECT == "PIXELATE") {
					sprites[spr].effects.pixelate += CHANGE;
				} else if (EFFECT == "MOSAIC") {
					sprites[spr].effects.mosaic += CHANGE;
				} else if (EFFECT == "BRIGHTNESS") {
					sprites[spr].effects.brightness += CHANGE;
				} else if (EFFECT == "GHOST") {
					sprites[spr].effects.ghost += CHANGE;
				}
			}  else if (thisBlock.opcode == "looks_seteffectto") {
				string EFFECT = thisBlock.fields["EFFECT"][0];
				float VALUE = dToNum(handleBlockInputs(spr, thisName, "VALUE"));
				if (EFFECT == "COLOR") {
					sprites[spr].effects.color = VALUE;
				} else if (EFFECT == "COLOR") {
					sprites[spr].effects.color = VALUE;
				} else if (EFFECT == "FISHEYE") {
					sprites[spr].effects.fisheye = VALUE;
				} else if (EFFECT == "WHIRL") {
					sprites[spr].effects.whirl = VALUE;
				} else if (EFFECT == "PIXELATE") {
					sprites[spr].effects.pixelate = VALUE;
				} else if (EFFECT == "MOSAIC") {
					sprites[spr].effects.mosaic = VALUE;
				} else if (EFFECT == "BRIGHTNESS") {
					sprites[spr].effects.brightness = VALUE;
				} else if (EFFECT == "GHOST") {
					sprites[spr].effects.ghost = VALUE;
				}
			} else if (thisBlock.opcode == "looks_cleargraphiceffects") {
				sprites[spr].effects.color = 0;
				sprites[spr].effects.fisheye = 0;
				sprites[spr].effects.whirl = 0;
				sprites[spr].effects.pixelate = 0;
				sprites[spr].effects.mosaic = 0;
				sprites[spr].effects.brightness = 0;
				sprites[spr].effects.ghost = 0;
			} else if (thisBlock.opcode == "looks_show") {
				sprites[spr].visible = true;
			} else if (thisBlock.opcode == "looks_hide") {
				sprites[spr].visible = false;
			} else if (thisBlock.opcode == "looks_gotofrontback") {
				string FRONT_BACK = thisBlock.fields["FRONT_BACK"][0];
				if (FRONT_BACK == "front") {
					int maxLayer = 0;
					for (int i = 0; i < sprites.size(); i++) {
						if (i == spr) continue;
						if (sprites[i].layerOrder > maxLayer) maxLayer = sprites[i].layerOrder;
						if (sprites[i].layerOrder > sprites[spr].layerOrder) sprites[i].layerOrder--;
					}
					sprites[spr].layerOrder = maxLayer;
				} else {
					for (int i = 1; i < sprites.size(); i++) {
						if (i == spr) continue;
						if (sprites[i].layerOrder < sprites[spr].layerOrder) sprites[i].layerOrder++;
					}
					sprites[spr].layerOrder = 1;
				}
			} else if (thisBlock.opcode == "looks_gotoforwardbackwardlayers") {
				string FORWARD_BACKWARD = thisBlock.fields["FORWARD_BACKWARD"][0];
				int NUM = ceil(dToNum(handleBlockInputs(spr, thisName, "NUM")));
				if (FORWARD_BACKWARD == "forward") {
					for (int i = 0; i < sprites.size(); i++) {
						if (i == spr) continue;
						if (sprites[i].layerOrder > sprites[spr].layerOrder && sprites[i].layerOrder < sprites[spr].layerOrder + NUM + 1) sprites[i].layerOrder--;
					}
					sprites[spr].layerOrder += NUM;
				} else {
					for (int i = 1; i < sprites.size(); i++) {
						if (i == spr) continue;
						if (sprites[i].layerOrder < sprites[spr].layerOrder && sprites[i].layerOrder > sprites[spr].layerOrder - NUM - 1) sprites[i].layerOrder++;
					}
					sprites[spr].layerOrder -= NUM;
				}
			}
		} else if (thisBlock.opcode[0] == 'c') {
			if (thisBlock.opcode == "control_wait") {
				if (sprites[spr].timer == 0) {
					float DURATION = dToNum(handleBlockInputs(spr, thisName, "DURATION"));
					sprites[spr].wait_duration = DURATION;
					sprites[spr].wait_startingTime = clock();
					sprites[spr].timer = 1;
				}
				float deltaTime = double(clock() - sprites[spr].wait_startingTime) / CLOCKS_PER_SEC / sprites[spr].wait_duration;
				if (deltaTime >= 1) {
					sprites[spr].timer = 0;
				}
			} else if (thisBlock.opcode == "control_repeat") {
				int TIMES = round(dToNum(handleBlockInputs(spr, thisName, "TIMES")));
				string SUBSTACK;
				try {
					SUBSTACK = thisBlock.inputs["SUBSTACK"][1];
				} catch (const json::exception& e) {
					return;
				}
				sprites[spr].nestStack.push_back(thisName);
				for (int i = 0; i < TIMES; i++) {
					currBlock = SUBSTACK;
					if (executeScript(spr) != 0) return;
				}
				currBlock = sprites[spr].nestStack[sprites[spr].nestStack.size() - 1];
				sprites[spr].nestStack.pop_back();
			} else if (thisBlock.opcode == "control_forever") {
				sprites[spr].nestStack.clear();
				string SUBSTACK = "null";
				try {
					SUBSTACK = thisBlock.inputs["SUBSTACK"][1];
				} catch (const json::exception& e) {}
				while (1) {
					if (SUBSTACK == "null") continue;
					currBlock = SUBSTACK;
					if (executeScript(spr) != 0) return;
				}
			} else if (thisBlock.opcode == "control_if") {
				string CONDITION = dToStr(handleBlockInputs(spr, thisName, "CONDITION"));
				string SUBSTACK;
				try {
					SUBSTACK = thisBlock.inputs["SUBSTACK"][1];
				} catch (const json::exception& e) {
					return;
				}
				sprites[spr].nestStack.push_back(thisName);
				if (CONDITION == "true") {
					currBlock = SUBSTACK;
					if (executeScript(spr) != 0) return;
				}
				currBlock = sprites[spr].nestStack[sprites[spr].nestStack.size() - 1];
				sprites[spr].nestStack.pop_back();
			} else if (thisBlock.opcode == "control_if_else") {
				string CONDITION = dToStr(handleBlockInputs(spr, thisName, "CONDITION"));
				string SUBSTACK = "null";
				string SUBSTACK2 = "null";
				try {
					SUBSTACK = thisBlock.inputs["SUBSTACK"][1];
				} catch (const json::exception& e) {}
				try {
					SUBSTACK2 = thisBlock.inputs["SUBSTACK2"][1];
				} catch (const json::exception& e) {}
				sprites[spr].nestStack.push_back(thisName);
				if (CONDITION == "true") {
					if (SUBSTACK != "null") {
						currBlock = SUBSTACK;
						if (executeScript(spr) != 0) return;
					}
				} else if (SUBSTACK2 != "null") {
					currBlock = SUBSTACK2;
					if (executeScript(spr) != 0) return;
				}
				currBlock = sprites[spr].nestStack[sprites[spr].nestStack.size() - 1];
				sprites[spr].nestStack.pop_back();
			} else if (thisBlock.opcode == "control_wait_until") {
				string CONDITION = dToStr(handleBlockInputs(spr, thisName, "CONDITION"));
				sprites[spr].timer = 1;
				if (CONDITION == "true") {
					sprites[spr].timer = 0;
				}
			} else if (thisBlock.opcode == "control_repeat_until") {
				string CONDITION = dToStr(handleBlockInputs(spr, thisName, "CONDITION"));
				string SUBSTACK;
				try {
					SUBSTACK = thisBlock.inputs["SUBSTACK"][1];
				} catch (const json::exception& e) {
					return;
				}
				sprites[spr].nestStack.push_back(thisName);
				while (CONDITION == "false") {
					currBlock = SUBSTACK;
					if (executeScript(spr) != 0) return;
					CONDITION = dToStr(handleBlockInputs(spr, thisName, "CONDITION"));
				}
				currBlock = sprites[spr].nestStack[sprites[spr].nestStack.size() - 1];
				sprites[spr].nestStack.pop_back();
			}
		} else if (thisBlock.opcode[0] == 's') {
			if (thisBlock.opcode == "sensing_askandwait") {
				string QUESTION = dToStr(handleBlockInputs(spr, thisName, "QUESTION"));
				sprites[spr].timer = 1;
				cout << "\n" << QUESTION << "\n";
				getline(cin, globalSens.answer);
				sprites[spr].timer = 0;
			} else if (thisBlock.opcode == "sensing_setdragmode") {
				string DRAG_MODE = thisBlock.fields["DRAG_MODE"][0];
				sprites[spr].draggable = DRAG_MODE == "draggable";
			} else if (thisBlock.opcode == "sensing_resettimer") {
				globalSens.timer = 0;
			}
		} else if (thisBlock.opcode[0] == 'd') {
			if (thisBlock.opcode == "data_setvariableto") {
				setVariable(spr, thisBlock.fields["VARIABLE"][1], dToStr(handleBlockInputs(spr, thisName, "VALUE")));
			} else if (thisBlock.opcode == "data_changevariableby") {
				float add1 = 0;
				try {
					add1 = dToNum(getVariable(spr, thisBlock.fields["VARIABLE"][1]));
				} catch (const exception& e) {}
				float add2 = 0;
				try {
					add2 = dToNum(handleBlockInputs(spr, thisName, "VALUE"));
				} catch (const exception& e) {}
				setVariable(spr, thisBlock.fields["VARIABLE"][1], to_string(add1 + add2));
			} else if (thisBlock.opcode == "data_addtolist") {
				getList(spr, thisBlock.fields["LIST"][1]).push_back(dToStr(handleBlockInputs(spr, thisName, "ITEM")));
			} else if (thisBlock.opcode == "data_deleteoflist") {
				int index = stof(dToStr(handleBlockInputs(spr, thisName, "INDEX")));
				if (index != 0) getList(spr, thisBlock.fields["LIST"][1]).erase(getList(spr, thisBlock.fields["LIST"][1]).begin() + index - 1);
			} else if (thisBlock.opcode == "data_deletealloflist") {
				getList(spr, thisBlock.fields["LIST"][1]).clear();
			} else if (thisBlock.opcode == "data_insertatlist") {
				getList(spr, thisBlock.fields["LIST"][1]).insert(getList(spr, thisBlock.fields["LIST"][1]).begin() + (int)stof(dToStr(handleBlockInputs(spr, thisName, "INDEX"))) - 1, dToStr(handleBlockInputs(spr, thisName, "ITEM")));
			} else if (thisBlock.opcode == "data_replaceitemoflist") {
				getList(spr, thisBlock.fields["LIST"][1])[stof(dToStr(handleBlockInputs(spr, thisName, "INDEX"))) - 1] = dToStr(handleBlockInputs(spr, thisName, "ITEM"));
			}
		}
	}

	bool executeScript (int spr) {
		while (sprites[spr].blocks[currBlock].next != "null") {
			handleBlocks(spr, currBlock);
			globalSens.timer = double(clock() - mariEng::programStart) / CLOCKS_PER_SEC;
			if (frameAdvance() != 0) return 1;
			if (sprites[spr].timer == 0) currBlock = sprites[spr].blocks[currBlock].next;
		}
		do {
			handleBlocks(spr, currBlock);
			if (frameAdvance() != 0) return 1;
		} while (sprites[spr].timer != 0);
		return 0;
	}

	string scratchSB3File = "";

	string getSb3FromZip () {
		if (greetings) {
			cout << "***Marigold the giant Cat - Scratch interpreter***";
			cout << "\n\nversion 0.1\n";
			cout << "\nPlease read Instructions.txt.\n";
			cout << "\nLooking for .sb3 project...\n";
		}
		for (const auto& dirEntry : recursive_directory_iterator("./")) {
			if (dirEntry.path().string().substr(dirEntry.path().string().size() - 4, 4) == ".sb3") {
				scratchSB3File = dirEntry.path().string();
				break;
			}
		}
		
		if (scratchSB3File == "") {
			cout << "Fatal error - Couldn't find .sb3 file.\nYou may now close the executable.";
			while (1) {}
		}
		
		/*if (greetings) cout << "Found file named " << scratchSB3File << ".\n";
		
		int err = 0;
		zip_t* za = zip_open(scratchSB3File.c_str(), 0, &err);
		if (!za) {
			cout << "Fatal error - Failed to open .sb3 file.\nYou may now close the executable.";
			while (1) {}
		}

		if (greetings) cout << "Looking for project.json file...\n";

		zip_stat_t st;
		if (zip_stat(za, "project.json", 0, &st) != 0) {
			zip_close(za);
			cout << "Fatal error - project.json not found in archive.\nYou may now close the executable.";
			while (1) {}
		}

		if (greetings) cout << "File found.\n";

		zip_file_t* zf = zip_fopen(za, "project.json", 0);
		if (!zf) {
			zip_close(za);
			cout << "Fatal error - Failed to open project.json.\nYou may now close the executable.";
			while (1) {}
		}

		std::string content;
		content.resize(st.size);

		zip_int64_t bytes_read = zip_fread(zf, content.data(), st.size);
		zip_fclose(zf);
		zip_close(za);

		if (bytes_read < 0) {
			cout << "Fatal error - Failed to read project.json.\nYou may now close the executable.";
			while (1) {}
		}
		
		return content;*/
		string command = "zipExtractor.exe \"" + scratchSB3File + "\" project.json";
		system(command.c_str());
		ifstream f("project.json");
		std::string ret;
		getline(f, ret);
		return ret;
	}

	int start () {
		//If this doesn't work comment this...
		string f = getSb3FromZip();
		
		if (greetings) cout << "Parsing contents...\n";
		//... and uncomment this
		//ifstream f("project.json");
		json data;
		try {
			data = json::parse(f);
		} catch (const json::exception& e) {
			cout << "Fatal error - Failed to parse json contents.\nYou may now close the executable.";
			while (1) {}
		}
		
		if (greetings) {
			cout << "Successfully parsed contents.\n";
			cout << "Extracting sprite data...\n";
		}
		
		//cout << data["targets"][1]["blocks"];

		for (auto i : data["targets"]) {
			Sprite thisSprite;
			thisSprite.name = i["name"];

			for (json::iterator it = i["blocks"].begin(); it != i["blocks"].end(); ++it) {
				thisSprite.blocks[it.key()].fields = it.value()["fields"];
				thisSprite.blocks[it.key()].inputs = it.value()["inputs"];
				thisSprite.blocks[it.key()].next = it.value()["next"] == nullptr ? "null" : it.value()["next"];
				thisSprite.blocks[it.key()].opcode = it.value()["opcode"];

				if (it.value()["opcode"] == "event_whenflagclicked") {
					thisSprite.event_wfc.push_back(it.key());
				}
			}

			for (json::iterator it = i["variables"].begin(); it != i["variables"].end(); ++it) {
				json res = it.value()[1];
				//TODO put this in lists too
				thisSprite.variables[it.key()] = res.type() == json::value_t::number_unsigned ? to_string(res.get<unsigned int>()) : (res.type() == json::value_t::number_integer ? to_string(res.get<int>()) : res.get<string>());
				try {
					thisSprite.variables[it.key()] = stof(get<string>(thisSprite.variables[it.key()]));
				} catch (const exception& e) {}
				thisSprite.varNames[it.value()[0]] = it.key();
			}

			for (json::iterator it = i["lists"].begin(); it != i["lists"].end(); ++it) {
				for (json thing : it.value()[1]) {
					string res = thing.type() == json::value_t::number_unsigned ? to_string(thing.get<unsigned int>()) : thing.get<string>();
					thisSprite.lists[it.key()].push_back(res);
				}
			}

			try {
				thisSprite.x = i["x"];
				thisSprite.y = i["y"];
				thisSprite.direction = i["direction"];
				thisSprite.rotationStyle = i["rotationStyle"];
			} catch (const json::exception& e) {
				thisSprite.direction = 90;
			}
			
			for (int j = 0; j < i["costumes"].size(); ++j) {
				json it = i["costumes"][j];
				Costume thisCostume;
				
				thisCostume.name = it["name"];
				thisCostume.md5ext = it["md5ext"];
				thisCostume.rotationCenterX = it["rotationCenterX"];
				thisCostume.rotationCenterY = it["rotationCenterY"];
				
				thisSprite.costumes.push_back(thisCostume);
			}
			
			thisSprite.currentCostume = i["currentCostume"];
			if (i["size"] != nullptr) thisSprite.size = i["size"];
			else thisSprite.size = 100;
			
			try {
				thisSprite.visible = i["visible"];
			} catch (const exception& e) {}
			
			thisSprite.layerOrder = i["layerOrder"];

			thisSprite.volume = i["volume"];
			try {
				thisSprite.draggable = i["draggable"];
			} catch (const json::exception& e) {}
			
			sprites.push_back(thisSprite);
		}
		
		if (greetings) cout << "Successfully extracted sprite data.\n";
		
		return 0;
	}
	
	void almostthere () {
		if (greetings) cout << "The project will now start. -Have fun!-\n";
	}
	
	void loadingTextures () {
		if (greetings) cout << "Loading textures...\n";
	}
	
	void texturesLoaded () {
		if (greetings) cout << "Successfully loaded textures.\n";
	}
	
	void goodbye () {
		if (greetings) {
			cout << "\n\nThe program successfully terminated!\nYou may now close the executable.";
		}
	}
	
	void greenFlag () {
		programStart = clock();
		frameAdvance();
		
		for (int i = 0; i < sprites.size(); i++) {
			for (int j = 0; j < sprites[i].event_wfc.size(); j++) {
				currBlock = sprites[i].blocks[sprites[i].event_wfc[j]].next;

				if (executeScript(i) != 0) return;
			}
		}
		while (frameAdvance() == 0) {}
		return;
	}
}