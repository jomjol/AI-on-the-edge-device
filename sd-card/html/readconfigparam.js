function readconfig_Version(){
     return "1.0.0 - 20200910";
 }

var config_gesamt;
var config_split;
var param;
var category;
var ref = new Array(2);

function ParseConfig() {
     config_split = config_gesamt.split("\n");
     var aktline = 0;

     param = new Object();
     category = new Object(); 

     var catname = "MakeImage";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "LogImageLocation");
     ParamAddValue(param, catname, "WaitBeforeTakingPicture");
     ParamAddValue(param, catname, "LogfileRetentionInDays");
     ParamAddValue(param, catname, "Brightness");
     ParamAddValue(param, catname, "Contrast");
     ParamAddValue(param, catname, "Saturation");
     ParamAddValue(param, catname, "ImageQuality");
     ParamAddValue(param, catname, "ImageSize");     
     ParamAddValue(param, catname, "FixedExposure");     

     var catname = "Alignment";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "InitialRotate");
     ParamAddValue(param, catname, "InitialMirror");
     ParamAddValue(param, catname, "SearchFieldX");
     ParamAddValue(param, catname, "SearchFieldY");     
     ParamAddValue(param, catname, "AlignmentAlgo");
     ParamAddValue(param, catname, "FlipImageSize");

     var catname = "Digits";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Model");
     ParamAddValue(param, catname, "LogImageLocation");
     ParamAddValue(param, catname, "LogfileRetentionInDays");
     ParamAddValue(param, catname, "ModelInputSize", 2);     

     var catname = "Analog";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Model");
     ParamAddValue(param, catname, "LogImageLocation");
     ParamAddValue(param, catname, "LogfileRetentionInDays");
     ParamAddValue(param, catname, "ModelInputSize", 2);
     ParamAddValue(param, catname, "ExtendedResolution");

     var catname = "PostProcessing";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "DecimalShift");
     ParamAddValue(param, catname, "PreValueUse");
     ParamAddValue(param, catname, "PreValueAgeStartup");
     ParamAddValue(param, catname, "AllowNegativeRates");
     ParamAddValue(param, catname, "MaxRateValue");
     ParamAddValue(param, catname, "ErrorMessage");
     ParamAddValue(param, catname, "CheckDigitIncreaseConsistency");     

     var catname = "MQTT";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Uri");
     ParamAddValue(param, catname, "Topic");
     ParamAddValue(param, catname, "TopicError");
     ParamAddValue(param, catname, "TopicRate");
     ParamAddValue(param, catname, "TopicTimeStamp");
     ParamAddValue(param, catname, "TopicUptime", 1, [/^([a-zA-Z0-9_-]+\/){0,10}[a-zA-Z0-9_-]+$/]);
     ParamAddValue(param, catname, "ClientID");
     ParamAddValue(param, catname, "user");
     ParamAddValue(param, catname, "password");
     
     var catname = "GPIO";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "MainTopicMQTT", 1, [/^([a-zA-Z0-9_-]+\/){0,10}[a-zA-Z0-9_-]+$/]);
     ParamAddValue(param, catname, "IO12", 6, [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);
     ParamAddValue(param, catname, "IO13", 6, [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);

     var catname = "AutoTimer";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "AutoStart");
     ParamAddValue(param, catname, "Intervall");     

     var catname = "Debug";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Logfile");
     ParamAddValue(param, catname, "LogfileRetentionInDays");

     var catname = "System";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "TimeZone");
     ParamAddValue(param, catname, "TimeServer");         
     ParamAddValue(param, catname, "AutoAdjustSummertime");
     ParamAddValue(param, catname, "Hostname");   
     ParamAddValue(param, catname, "SetupMode");   

     while (aktline < config_split.length){
          for (var cat in category) {
               zw = cat.toUpperCase();
               zw1 = "[" + zw + "]";
               zw2 = ";[" + zw + "]";
               if ((config_split[aktline].trim().toUpperCase() == zw1) || (config_split[aktline].trim().toUpperCase() == zw2)) {
                    if (config_split[aktline].trim().toUpperCase() == zw1) {
                         category[cat]["enabled"] = true;
                    }
                    category[cat]["found"] = true;
                    category[cat]["line"] = aktline;
                    aktline = ParseConfigParamAll(aktline, cat);
                    continue;
               }
          }
          aktline++;
     }
}

function ParamAddValue(param, _cat, _param, _anzParam = 1, _checkRegExList = null){
     param[_cat][_param] = new Object(); 
     param[_cat][_param]["found"] = false;
     param[_cat][_param]["enabled"] = false;
     param[_cat][_param]["line"] = -1; 
     param[_cat][_param]["anzParam"] = _anzParam;
     param[_cat][_param].checkRegExList = _checkRegExList;
};



function ParseConfigParamAll(_aktline, _catname){
     ++_aktline;

     while ((_aktline < config_split.length) 
            && !(config_split[_aktline][0] == "[") 
            && !((config_split[_aktline][0] == ";") && (config_split[_aktline][1] == "["))) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);
          ParamExtractValueAll(param, linesplit, _catname, _aktline, isCom);

          ++_aktline;
     }    
     return _aktline; 
}

function ParamExtractValue(_param, _linesplit, _catname, _paramname, _aktline, _iscom, _anzvalue = 1){
     if ((_linesplit[0].toUpperCase() == _paramname.toUpperCase()) && (_linesplit.length > _anzvalue))
     {
          _param[_catname][_paramname]["found"] = true;
          _param[_catname][_paramname]["enabled"] = !_iscom;
          _param[_catname][_paramname]["line"] = _aktline;
          _param[_catname][_paramname]["anzpara"] = _anzvalue;
          for (var j = 1; j <= _anzvalue; ++j) {
               _param[_catname][_paramname]["value"+j] = _linesplit[j];
               }
     }
}

function ParamExtractValueAll(_param, _linesplit, _catname, _aktline, _iscom){
     for (var paramname in _param[_catname]) {
          if (_linesplit[0].toUpperCase() == paramname.toUpperCase())
          {
               while (_linesplit.length <= _param[_catname][paramname]["anzParam"]) {
                    _linesplit.push("");
               }

               _param[_catname][paramname]["found"] = true;
               _param[_catname][paramname]["enabled"] = !_iscom;
               _param[_catname][paramname]["line"] = _aktline;
               for (var j = 1; j <= _param[_catname][paramname]["anzParam"]; ++j) {
                    _param[_catname][paramname]["value"+j] = _linesplit[j];
                    }
          }
     }
}

function getConfigParameters() {
     return param;
}

function setConfigParameters(_param, _category = "") {
     for (var cat in _param) {
          for (var name in _param[cat]) {
               param[cat][name]["found"] = _param[cat][name]["found"];
               param[cat][name]["enabled"] = _param[cat][name]["enabled"];
               param[cat][name]["line"] = _param[cat][name]["line"];

               param[cat][name]["anzParam"] = _param[cat][name]["anzParam"];
               for (var j = 1; j <= _param[cat][name]["anzParam"]; ++j) {
                    param[cat][name]["value"+j] =  _param[cat][name]["value"+j];
                    }

               if (param[cat][name]["found"]) {
                    var text = name + " =" 
                    
                    for (var j = 1; j <= _param[cat][name]["anzParam"]; ++j) {
                         text = text + " " + param[cat][name]["value"+j];
                         }
                    if (!param[cat][name]["enabled"]) {
                         text = ";" + text;
                    }
                    config_split[param[cat][name]["line"]] = text;
               }
          }
     }

     for (var cat in _category) {
          if (category[cat]["found"])
          {
               category[cat]["enabled"] = _category[cat]["enabled"];
               text = "[" + cat + "]";
               if (!category[cat]["enabled"]) {
                    text = ";" + text;
               }
               config_split[category[cat]["line"]] = text;

          }
     }
     
     config_gesamt = config_split[0];
     for (var i = 1; i < config_split.length; ++i){
          config_gesamt = config_gesamt + "\n" + config_split[i]; 
     }

     return config_gesamt;
}


function isCommented(input)
     {
          let isComment = false;
          if (input.charAt(0) == ';') {
               isComment = true;
               input = input.substr(1, input.length-1);
          };
          return [isComment, input];
     }    

function SaveConfigToServer(_basepath){
     // leere Zeilen am Ende löschen
     var zw = config_split.length - 1;
     while (config_split[zw] == "") {
          config_split.pop();
     }

     var config_gesamt = "";
     for (var i = 0; i < config_split.length; ++i)
     {
          config_gesamt = config_gesamt + config_split[i] + "\n";
     } 

     FileDeleteOnServer("/config/config.ini", _basepath);
     FileSendContent(config_gesamt, "/config/config.ini", _basepath);          
}
	 
function getConfig() {
	return config_gesamt;
     }

function getConfigCategory() {
     return category;
}
     


