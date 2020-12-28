function readconfig_Version(){
     return "1.0.0 - 20200910";
 }

var config_gesamt;
var config_split;
var param;
var ref = new Array(2);

function ParseConfig() {
     config_split = config_gesamt.split("\n");
     var aktline = 0;

     param = new Object();

     var catname = "MakeImage";
     param[catname] = new Object();
     ParamAddValue(param, catname, "LogImageLocation");
     ParamAddValue(param, catname, "WaitBeforeTakingPicture");
     ParamAddValue(param, catname, "LogfileRetentionInDays");
     ParamAddValue(param, catname, "ImageQuality");
     ParamAddValue(param, catname, "ImageSize");     

     var catname = "Alignment";
     param[catname] = new Object();
     ParamAddValue(param, catname, "SearchFieldX");
     ParamAddValue(param, catname, "SearchFieldY");     

     var catname = "Digits";
     param[catname] = new Object();
     ParamAddValue(param, catname, "Model");
     ParamAddValue(param, catname, "LogImageLocation");
     ParamAddValue(param, catname, "LogfileRetentionInDays");
     ParamAddValue(param, catname, "ModelInputSize");     

     var catname = "Analog";
     param[catname] = new Object();
     ParamAddValue(param, catname, "Model");
     ParamAddValue(param, catname, "LogImageLocation");
     ParamAddValue(param, catname, "LogfileRetentionInDays");
     ParamAddValue(param, catname, "ModelInputSize");

     var catname = "PostProcessing";
     param[catname] = new Object();
     ParamAddValue(param, catname, "DecimalShift");
     ParamAddValue(param, catname, "PreValueUse");
     ParamAddValue(param, catname, "PreValueAgeStartup");
     ParamAddValue(param, catname, "AllowNegativeRates");
     ParamAddValue(param, catname, "MaxRateValue");
     ParamAddValue(param, catname, "ErrorMessage");
     ParamAddValue(param, catname, "CheckDigitIncreaseConsistency");     

     var catname = "MQTT";
     param[catname] = new Object();
     ParamAddValue(param, catname, "Uri");
     ParamAddValue(param, catname, "Topic");
     ParamAddValue(param, catname, "TopicError");
     ParamAddValue(param, catname, "ClientID");
     ParamAddValue(param, catname, "user");
     ParamAddValue(param, catname, "password");     

     var catname = "AutoTimer";
     param[catname] = new Object();
     ParamAddValue(param, catname, "AutoStart");
     ParamAddValue(param, catname, "Intervall");     

     var catname = "Debug";
     param[catname] = new Object();
     ParamAddValue(param, catname, "Logfile");
     ParamAddValue(param, catname, "LogfileRetentionInDays");

     var catname = "System";
     param[catname] = new Object();
     ParamAddValue(param, catname, "TimeZone");
     ParamAddValue(param, catname, "TimeServer");         
     ParamAddValue(param, catname, "AutoAdjustSummertime");
     ParamAddValue(param, catname, "SetupMode");   

     while (aktline < config_split.length){
          if (config_split[aktline].trim().toUpperCase() == "[MAKEIMAGE]") {
               aktline = ParseConfigParamMakeImage(aktline);
               continue;
          }

          if (config_split[aktline].trim().toUpperCase() == "[ALIGNMENT]") {
               aktline = ParseConfigParamAlignment(aktline);
               continue;
          }

          if (config_split[aktline].trim().toUpperCase() == "[DIGITS]") {
               aktline = ParseConfigParamDigit(aktline);
               continue;
          }

          if (config_split[aktline].trim().toUpperCase() == "[ANALOG]") {
               aktline = ParseConfigParamAnalog(aktline);
               continue;
          }

          if (config_split[aktline].trim().toUpperCase() == "[POSTPROCESSING]") {
               aktline = ParseConfigParamPostProcessing(aktline);
               continue;
          }          

          if (config_split[aktline].trim().toUpperCase() == "[MQTT]") {
               aktline = ParseConfigParamMQTT(aktline);
               continue;
          }  

          if (config_split[aktline].trim().toUpperCase() == "[AUTOTIMER]") {
               aktline = ParseConfigParamAutoTimer(aktline);
               continue;
          }  

          if (config_split[aktline].trim().toUpperCase() == "[DEBUG]") {
               aktline = ParseConfigParamDebug(aktline);
               continue;
          }          

          if (config_split[aktline].trim().toUpperCase() == "[SYSTEM]") {
               aktline = ParseConfigParamSystem(aktline);
               continue;
          }              
          
          



          aktline++;
     }
}

function ParamAddValue(param, _cat, _param){
     param[_cat][_param] = new Object(); 
     param[_cat][_param]["found"] = false;
     param[_cat][_param]["enabled"] = false;
     param[_cat][_param]["line"] = -1;     
};


function ParseConfigParamSystem(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "System";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input, " =");

          ParamExtractValue(param, linesplit, catname, "TimeZone", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "TimeServer", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "AutoAdjustSummertime", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "TimeUpdateIntervall", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "SetupMode", _aktline, isCom);

          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigParamDebug(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "Debug";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "Logfile", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "LogfileRetentionInDays", _aktline, isCom);

          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigParamAutoTimer(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "AutoTimer";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "AutoStart", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "Intervall", _aktline, isCom);

          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigParamMQTT(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "MQTT";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "Uri", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "Topic", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "TopicError", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "ClientID", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "user", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "password", _aktline, isCom);

          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigParamPostProcessing(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "PostProcessing";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "DecimalShift", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "PreValueUse", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "PreValueAgeStartup", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "AllowNegativeRates", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "MaxRateValue", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "ErrorMessage", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "CheckDigitIncreaseConsistency", _aktline, isCom);

          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigParamAnalog(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "Analog";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "Model", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "LogImageLocation", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "LogfileRetentionInDays", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "ModelInputSize", _aktline, isCom, 2);

          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigParamDigit(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "Digits";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "Model", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "LogImageLocation", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "LogfileRetentionInDays", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "ModelInputSize", _aktline, isCom, 2);

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


function ParseConfigParamAlignment(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "Alignment";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "SearchFieldX", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "SearchFieldY", _aktline, isCom);

          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigParamMakeImage(_aktline){
     var akt_ref = 0;
     ++_aktline;

     var catname = "MakeImage";
     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
          var _input = config_split[_aktline];
          let [isCom, input] = isCommented(_input);
          var linesplit = ZerlegeZeile(input);

          ParamExtractValue(param, linesplit, catname, "LogImageLocation", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "WaitBeforeTakingPicture", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "LogfileRetentionInDays", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "ImageQuality", _aktline, isCom);
          ParamExtractValue(param, linesplit, catname, "ImageSize", _aktline, isCom);          

          ++_aktline;
     }    
     return _aktline; 
}

function getConfigParameters() {
     return param;
}

function setConfigParameters(_param) {
     for (var cat in _param) {
          for (var name in _param[cat]) {
               param[cat][name]["found"] = _param[cat][name]["found"];
               param[cat][name]["enabled"] = _param[cat][name]["enabled"];
               param[cat][name]["line"] = _param[cat][name]["line"];

               param[cat][name]["anzpara"] = _param[cat][name]["anzpara"];
               for (var j = 1; j <= _param[cat][name]["anzpara"]; ++j) {
                    param[cat][name]["value"+j] =  _param[cat][name]["value"+j];
                    }

               if (param[cat][name]["found"]) {
                    var text = name + " =" 
                    
                    for (var j = 1; j <= _param[cat][name]["anzpara"]; ++j) {
                         text = text + " " + param[cat][name]["value"+j];
                         }
                    if (!param[cat][name]["enabled"]) {
                         text = ";" + text;
                    }
                    config_split[param[cat][name]["line"]] = text;
               }
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

function createReader(file) {
     var image = new Image();
     reader.onload = function(evt) {
         var image = new Image();
         image.onload = function(evt) {
             var width = this.width;
             var height = this.height;
             alert (width); // will produce something like 198
         };
         image.src = evt.target.result; 
     };
     reader.readAsDataURL(file);
 }

function ZerlegeZeile(input, delimiter = " =,")
     {
          var Output = Array(0);
//          delimiter = " =,";
     
          input = trim(input, delimiter);
          var pos = findDelimiterPos(input, delimiter);
          var token;
          while (pos > -1) {
               token = input.substr(0, pos);
               token = trim(token, delimiter);
               Output.push(token);
               input = input.substr(pos+1, input.length);
               input = trim(input, delimiter);
               pos = findDelimiterPos(input, delimiter);
          }
          Output.push(input);
     
          return Output;
     
     }    

function findDelimiterPos(input, delimiter)
     {
          var pos = -1;
          var zw;
          var akt_del;
     
          for (var anz = 0; anz < delimiter.length; ++anz)
          {
               akt_del = delimiter[anz];
               zw = input.indexOf(akt_del);
               if (zw > -1)
               {
                    if (pos > -1)
                    {
                         if (zw < pos)
                              pos = zw;
                    }
                    else
                         pos = zw;
               }
          }
          return pos;
     }
 
function trim(istring, adddelimiter)
     {
          while ((istring.length > 0) && (adddelimiter.indexOf(istring[0]) >= 0)){
               istring = istring.substr(1, istring.length-1);
          }
          
          while ((istring.length > 0) && (adddelimiter.indexOf(istring[istring.length-1]) >= 0)){
               istring = istring.substr(0, istring.length-1);
          }

          return istring;
     }
     
function loadConfig(_basepath) {
     var xhttp = new XMLHttpRequest();
     config_gesamt = "";
     try {
          url = _basepath + '/fileserver/config/config.ini';     
          xhttp.open("GET", url, false);
          xhttp.send();
          config_gesamt = xhttp.responseText;
          return true;
     }
     catch (error)
     {
     //          alert("Deleting Config.ini failed");
     }
     return false;
}
	 
function getConfig() {
	return config_gesamt;
     }
     
     
function FileCopyOnServer(_source, _target, _basepath = ""){
     url = _basepath + "/editflow.html?task=copy&in=" + _source + "&out=" + _target;
     var xhttp = new XMLHttpRequest();  
     try {
          xhttp.open("GET", url, false);
          xhttp.send();     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }
}

function FileDeleteOnServer(_filename, _basepath = ""){
     var xhttp = new XMLHttpRequest();
     var okay = false;

     xhttp.onreadystatechange = function() {
          if (xhttp.readyState == 4) {
               if (xhttp.status == 200) {
                    okay = true;
               } else if (xhttp.status == 0) {
//                    alert("Server closed the connection on delete abruptly!");
//                    location.reload()
               } else {
//                    alert(xhttp.status + " Error!\n" + xhttp.responseText);
//                    location.reload()
               }
          }
     };
     try {
          var url = _basepath + "/delete" + _filename;
          xhttp.open("POST", url, false);
          xhttp.send();
     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }

     return okay;
}

function FileSendContent(_content, _filename, _basepath = ""){
     var xhttp = new XMLHttpRequest();  
     var okay = false;

     xhttp.onreadystatechange = function() {
          if (xhttp.readyState == 4) {
               if (xhttp.status == 200) {
                    okay = true;
               } else if (xhttp.status == 0) {
                    alert("Server closed the connection abruptly!");
               } else {
                    alert(xhttp.status + " Error!\n" + xhttp.responseText);
               }
          }
     };

     try {
          upload_path = _basepath + "/upload" + _filename;
          xhttp.open("POST", upload_path, false);
          xhttp.send(_content);
     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }     
    return okay;        
}
