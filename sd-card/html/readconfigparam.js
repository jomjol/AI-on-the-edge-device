var config_gesamt = "";
var config_split = [];
var param = [];
var category;
var ref = new Array(2);
var NUMBERS = new Array(0);
var REFERENCES = new Array(0);


function getNUMBERSList() {
	_domainname = getDomainname(); 
     var namenumberslist = "";

	var xhttp = new XMLHttpRequest();
	xhttp.addEventListener('load', function(event) {
          if (xhttp.status >= 200 && xhttp.status < 300) {
               namenumberslist = xhttp.responseText;
          } else {
               console.warn(request.statusText, request.responseText);
          }
     });

     try {
          url = _domainname + '/editflow?task=namenumbers';     
          xhttp.open("GET", url, false);
          xhttp.send();
     }
     catch (error)
     {
//               alert("Loading Hostname failed");
     }

     namenumberslist = namenumberslist.split("\t");
//      namenumberslist.pop();

     return namenumberslist;
}


function getDATAList() {
	_domainname = getDomainname(); 
     datalist = "";

	var xhttp = new XMLHttpRequest();
	xhttp.addEventListener('load', function(event) {
          if (xhttp.status >= 200 && xhttp.status < 300) {
               datalist = xhttp.responseText;
          } else {
               console.warn(request.statusText, request.responseText);
          }
     });

     try {
          url = _domainname + '/editflow?task=data';     
          xhttp.open("GET", url, false);
          xhttp.send();
     }
     catch (error)
     {
//               alert("Loading Hostname failed");
     }

     datalist = datalist.split("\t");
     datalist.pop();
     datalist.sort();

     return datalist;
}


function getTFLITEList() {
	_domainname = getDomainname(); 
     tflitelist = "";

	var xhttp = new XMLHttpRequest();
	xhttp.addEventListener('load', function(event) {
          if (xhttp.status >= 200 && xhttp.status < 300) {
               tflitelist = xhttp.responseText;
          } else {
               console.warn(request.statusText, request.responseText);
          }
     });

     try {
          url = _domainname + '/editflow?task=tflite';
          xhttp.open("GET", url, false);
          xhttp.send();
     }
     catch (error)
     {
//               alert("Loading Hostname failed");
     }

     tflitelist = tflitelist.split("\t");
     tflitelist.sort();

     return tflitelist;
}


function ParseConfig() {
     config_split = config_gesamt.split("\n");
     var aktline = 0;

     param = new Object();
     category = new Object(); 

     var catname = "TakeImage";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "RawImagesLocation");
     ParamAddValue(param, catname, "WaitBeforeTakingPicture");
     ParamAddValue(param, catname, "RawImagesRetention");
     ParamAddValue(param, catname, "Demo");
     ParamAddValue(param, catname, "Brightness");
     ParamAddValue(param, catname, "Contrast");
     ParamAddValue(param, catname, "Saturation");
     ParamAddValue(param, catname, "LEDIntensity");
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
     ParamAddValue(param, catname, "CNNGoodThreshold", 1); 
     ParamAddValue(param, catname, "ROIImagesLocation");
     ParamAddValue(param, catname, "ROIImagesRetention");

     var catname = "Analog";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Model");
     ParamAddValue(param, catname, "ROIImagesLocation");
     ParamAddValue(param, catname, "ROIImagesRetention");

     var catname = "PostProcessing";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "DecimalShift", 1, true);
     ParamAddValue(param, catname, "AnalogDigitalTransitionStart", 1, true);
     ParamAddValue(param, catname, "PreValueUse");
     ParamAddValue(param, catname, "PreValueAgeStartup");
     ParamAddValue(param, catname, "AllowNegativeRates", 1, true, "true");
     ParamAddValue(param, catname, "MaxRateValue", 1, true);
     ParamAddValue(param, catname, "MaxRateType", 1, true);
     ParamAddValue(param, catname, "ExtendedResolution", 1, true, "false");
     ParamAddValue(param, catname, "IgnoreLeadingNaN", 1, true, "false");
     ParamAddValue(param, catname, "ErrorMessage");
     ParamAddValue(param, catname, "CheckDigitIncreaseConsistency");     

     var catname = "MQTT";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Uri");
     ParamAddValue(param, catname, "MainTopic", 1, false);
     ParamAddValue(param, catname, "ClientID");
     ParamAddValue(param, catname, "user");
     ParamAddValue(param, catname, "password");
     ParamAddValue(param, catname, "RetainMessages");
     ParamAddValue(param, catname, "HomeassistantDiscovery");
     ParamAddValue(param, catname, "MeterType");
     ParamAddValue(param, catname, "CACert");
     ParamAddValue(param, catname, "ClientCert");
     ParamAddValue(param, catname, "ClientKey");

     var catname = "InfluxDB";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Uri");
     ParamAddValue(param, catname, "Database");
//     ParamAddValue(param, catname, "Measurement");
     ParamAddValue(param, catname, "user");
     ParamAddValue(param, catname, "password");
     ParamAddValue(param, catname, "Measurement", 1, true);
     ParamAddValue(param, catname, "Field", 1, true);

     var catname = "InfluxDBv2";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "Uri");
     ParamAddValue(param, catname, "Database");
//     ParamAddValue(param, catname, "Measurement");
     ParamAddValue(param, catname, "Org");
     ParamAddValue(param, catname, "Token");
     ParamAddValue(param, catname, "Measurement", 1, true);
     ParamAddValue(param, catname, "Field", 1, true);

     var catname = "GPIO";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "IO0", 6, false, "", [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);
     ParamAddValue(param, catname, "IO1", 6, false, "",  [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);
     ParamAddValue(param, catname, "IO3", 6, false, "",  [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);
     ParamAddValue(param, catname, "IO4", 6, false, "",  [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);
     ParamAddValue(param, catname, "IO12", 6, false, "",  [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);
     ParamAddValue(param, catname, "IO13", 6, false, "",  [null, null, /^[0-9]*$/, null, null, /^[a-zA-Z0-9_-]*$/]);
     ParamAddValue(param, catname, "LEDType");
     ParamAddValue(param, catname, "LEDNumbers");
     ParamAddValue(param, catname, "LEDColor", 3);
     // Default Values, um abwärtskompatiblität zu gewährleisten
     param[catname]["LEDType"]["value1"] = "WS2812";
     param[catname]["LEDNumbers"]["value1"] = "2";
     param[catname]["LEDColor"]["value1"] = "50";
     param[catname]["LEDColor"]["value2"] = "50";
     param[catname]["LEDColor"]["value3"] = "50";


     var catname = "AutoTimer";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "AutoStart");
     ParamAddValue(param, catname, "Interval");     

     var catname = "DataLogging";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "DataLogActive");
     ParamAddValue(param, catname, "DataFilesRetention");     

     var catname = "Debug";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "LogLevel");
     ParamAddValue(param, catname, "LogfilesRetention");

     var catname = "System";
     category[catname] = new Object(); 
     category[catname]["enabled"] = false;
     category[catname]["found"] = false;
     param[catname] = new Object();
     ParamAddValue(param, catname, "TimeZone");
     ParamAddValue(param, catname, "TimeServer");         
     ParamAddValue(param, catname, "Hostname");   
     ParamAddValue(param, catname, "RSSIThreshold");   
     ParamAddValue(param, catname, "CPUFrequency");
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

     // Make the downward compatiblity with DataLogging
     if (category["DataLogging"]["found"] == false)
     {
          category["DataLogging"]["found"] = true;
          category["DataLogging"]["enabled"] = true;

          param["DataLogging"]["DataLogActive"]["found"] = true;
          param["DataLogging"]["DataLogActive"]["enabled"] = true;
          param["DataLogging"]["DataLogActive"]["value1"] = "true";
          
          param["DataLogging"]["DataFilesRetention"]["found"] = true;
          param["DataLogging"]["DataFilesRetention"]["enabled"] = true;
          param["DataLogging"]["DataFilesRetention"]["value1"] = "3";
     }

     if (category["DataLogging"]["enabled"] == false)
          category["DataLogging"]["enabled"] = true

     if (param["DataLogging"]["DataLogActive"]["enabled"] == false && param["DataLogging"]["DataLogActive"]["value1"] == "")
     {
          param["DataLogging"]["DataLogActive"]["found"] = true;
          param["DataLogging"]["DataLogActive"]["enabled"] = true;
          param["DataLogging"]["DataLogActive"]["value1"] = "true";
     }

     if (param["DataLogging"]["DataFilesRetention"]["enabled"] == false && param["DataLogging"]["DataFilesRetention"]["value1"] == "")
     {
          param["DataLogging"]["DataFilesRetention"]["found"] = true;
          param["DataLogging"]["DataFilesRetention"]["enabled"] = true;
          param["DataLogging"]["DataFilesRetention"]["value1"] = "3";
     }

     // Downward compatibility: Create RSSIThreshold if not available
     if (param["System"]["RSSIThreshold"]["found"] == false)
     {
          param["System"]["RSSIThreshold"]["found"] = true;
          param["System"]["RSSIThreshold"]["enabled"] = false;
          param["System"]["RSSIThreshold"]["value1"] = "0";
     }
}


function ParamAddValue(param, _cat, _param, _anzParam = 1, _isNUMBER = false, _defaultValue = "", _checkRegExList = null){
     param[_cat][_param] = new Object(); 
     param[_cat][_param]["found"] = false;
     param[_cat][_param]["enabled"] = false;
     param[_cat][_param]["line"] = -1; 
     param[_cat][_param]["anzParam"] = _anzParam;
     param[_cat][_param]["defaultValue"] = _defaultValue;
     param[_cat][_param]["Numbers"] = _isNUMBER;
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
          if (!isCom && (linesplit.length >= 5) && (_catname == 'Digits'))
               ExtractROIs(input, "digit");
          if (!isCom && (linesplit.length >= 5) && (_catname == 'Analog'))
               ExtractROIs(input, "analog");
          if (!isCom && (linesplit.length == 3) && (_catname == 'Alignment'))
          {
               _newref = new Object();
               _newref["name"] = linesplit[0];
               _newref["x"] = linesplit[1];
               _newref["y"] = linesplit[2];
               REFERENCES.push(_newref);
          }

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
          _AktROI = "default";
          _AktPara = _linesplit[0];
          _pospunkt = _AktPara.indexOf (".");
          if (_pospunkt > -1)
          {
               _AktROI = _AktPara.substring(0, _pospunkt);
               _AktPara = _AktPara.substring(_pospunkt+1);
          }
          if (_AktPara.toUpperCase() == paramname.toUpperCase())
          {
               while (_linesplit.length <= _param[_catname][paramname]["anzParam"]) {
                    _linesplit.push("");
               }

               _param[_catname][paramname]["found"] = true;
               _param[_catname][paramname]["enabled"] = !_iscom;
               _param[_catname][paramname]["line"] = _aktline;
               if (_param[_catname][paramname]["Numbers"] == true)         // möglicher Multiusage
               {
                    abc = getNUMBERS(_linesplit[0]);
                    abc[_catname][paramname] = new Object;
                    abc[_catname][paramname]["found"] = true;
                    abc[_catname][paramname]["enabled"] = !_iscom;
     
                    for (var j = 1; j <= _param[_catname][paramname]["anzParam"]; ++j) {
                         abc[_catname][paramname]["value"+j] = _linesplit[j];
                         }
                    if (abc["name"] == "default")
                    {
                    for (_num in NUMBERS)         // wert mit Default belegen
                         {
                              if (NUMBERS[_num][_catname][paramname]["found"] == false)
                              {
                                   NUMBERS[_num][_catname][paramname]["found"] = true;
                                   NUMBERS[_num][_catname][paramname]["enabled"] = !_iscom;
                                   NUMBERS[_num][_catname][paramname]["line"] = _aktline;
                                   for (var j = 1; j <= _param[_catname][paramname]["anzParam"]; ++j) {
                                        NUMBERS[_num][_catname][paramname]["value"+j] = _linesplit[j];
                                        }

                              }
                         }
                    }
               }
               else
               {
                    _param[_catname][paramname]["found"] = true;
                    _param[_catname][paramname]["enabled"] = !_iscom;
                    _param[_catname][paramname]["line"] = _aktline;
                         for (var j = 1; j <= _param[_catname][paramname]["anzParam"]; ++j) {
                         _param[_catname][paramname]["value"+j] = _linesplit[j];
                         }
               }
          }
     }
}

function getConfigParameters() {
     return param;
}

function WriteConfigININew()
{
     // Cleanup empty NUMBERS
     for (var j = 0; j < NUMBERS.length; ++j)
     {
          if ((NUMBERS[j]["digit"].length + NUMBERS[j]["analog"].length) == 0)
          {
               NUMBERS.splice(j, 1);
          }
     }



     config_split = new Array(0);

     for (var cat in param) {
          text = "[" + cat + "]";
          if (!category[cat]["enabled"]) {
               text = ";" + text;
          }
          config_split.push(text);

          for (var name in param[cat]) {
               if (param[cat][name]["Numbers"])
               {
                    for (_num in NUMBERS)
                    {
                         text = NUMBERS[_num]["name"] + "." + name;

                         var text = text + " =" 
                         
                         for (var j = 1; j <= param[cat][name]["anzParam"]; ++j) {
                              if (!(typeof NUMBERS[_num][cat][name]["value"+j] == 'undefined'))
                                   text = text + " " + NUMBERS[_num][cat][name]["value"+j];
                              }
                         if (!NUMBERS[_num][cat][name]["enabled"]) {
                              text = ";" + text;
                         }
                         config_split.push(text);
                    }
               }
               else
               {
                    var text = name + " =" 
                    
                    for (var j = 1; j <= param[cat][name]["anzParam"]; ++j) {
                         if (!(typeof param[cat][name]["value"+j] == 'undefined'))
                              text = text + " " + param[cat][name]["value"+j];
                         }
                    if (!param[cat][name]["enabled"]) {
                         text = ";" + text;
                    }
                    config_split.push(text);
               }
          }
          if (cat == "Digits")
          {
               for (var _roi in NUMBERS)
               {
                    if (NUMBERS[_roi]["digit"].length > 0)
                    {
                         for (var _roiddet in NUMBERS[_roi]["digit"])
                         {
                              text = NUMBERS[_roi]["name"] + "." + NUMBERS[_roi]["digit"][_roiddet]["name"];
                              text = text + " " + NUMBERS[_roi]["digit"][_roiddet]["x"];
                              text = text + " " + NUMBERS[_roi]["digit"][_roiddet]["y"];
                              text = text + " " + NUMBERS[_roi]["digit"][_roiddet]["dx"];
                              text = text + " " + NUMBERS[_roi]["digit"][_roiddet]["dy"];
                              text = text + " " + NUMBERS[_roi]["digit"][_roiddet]["CCW"];
                              config_split.push(text);
                         }
                    }
               }
          }
          if (cat == "Analog")
          {
               for (var _roi in NUMBERS)
               {
                    if (NUMBERS[_roi]["analog"].length > 0)
                    {
                         for (var _roiddet in NUMBERS[_roi]["analog"])
                         {
                              text = NUMBERS[_roi]["name"] + "." + NUMBERS[_roi]["analog"][_roiddet]["name"];
                              text = text + " " + NUMBERS[_roi]["analog"][_roiddet]["x"];
                              text = text + " " + NUMBERS[_roi]["analog"][_roiddet]["y"];
                              text = text + " " + NUMBERS[_roi]["analog"][_roiddet]["dx"];
                              text = text + " " + NUMBERS[_roi]["analog"][_roiddet]["dy"];
                              text = text + " " + NUMBERS[_roi]["analog"][_roiddet]["CCW"];
                              config_split.push(text);
                         }
                    }
               }
          }
          if (cat == "Alignment")
          {
               for (var _roi in REFERENCES)
               {
                    text = REFERENCES[_roi]["name"];
                    text = text + " " + REFERENCES[_roi]["x"];
                    text = text + " " + REFERENCES[_roi]["y"];
                    config_split.push(text);
               }
          }

          config_split.push("");
     }
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

function SaveConfigToServer(_domainname){
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

     FileDeleteOnServer("/config/config.ini", _domainname);
     FileSendContent(config_gesamt, "/config/config.ini", _domainname);          
}
	 
function getConfig() {
	return config_gesamt;
     }

function getConfigCategory() {
     return category;
}



function ExtractROIs(_aktline, _type){
     var linesplit = ZerlegeZeile(_aktline);
     abc = getNUMBERS(linesplit[0], _type);
     abc["pos_ref"] = _aktline;
     abc["x"] = linesplit[1];
     abc["y"] = linesplit[2];
     abc["dx"] = linesplit[3];
     abc["dy"] = linesplit[4];
     abc["ar"] = parseFloat(linesplit[3]) / parseFloat(linesplit[4]);
     abc["CCW"] = "false";
     if (linesplit.length >= 6)
          abc["CCW"] = linesplit[5];
}


function getNUMBERS(_name, _type, _create = true)
{
     _pospunkt = _name.indexOf (".");
     if (_pospunkt > -1)
     {
          _digit = _name.substring(0, _pospunkt);
          _roi = _name.substring(_pospunkt+1);
     }
     else
     {
          _digit = "default";
          _roi = _name;
     }

     _ret = -1;

     for (i = 0; i < NUMBERS.length; ++i)
     {
          if (NUMBERS[i]["name"] == _digit)
               _ret = NUMBERS[i];
     }

     if (!_create)         // nicht gefunden und soll auch nicht erzeugt werden, ggf. geht eine NULL zurück
          return _ret;

     if (_ret == -1)
     {
          _ret = new Object();
          _ret["name"] = _digit;
          _ret['digit'] = new Array();
          _ret['analog'] = new Array();

          for (_cat in param)
               for (_param in param[_cat])
                    if (param[_cat][_param]["Numbers"] == true){
                         if (typeof  _ret[_cat] == 'undefined')
                              _ret[_cat] = new Object();
                         _ret[_cat][_param] = new Object();
                         _ret[_cat][_param]["found"] = false;
                         _ret[_cat][_param]["enabled"] = false;
                         _ret[_cat][_param]["anzParam"] = param[_cat][_param]["anzParam"]; 

                    }

          NUMBERS.push(_ret);
     }

     if (typeof _type == 'undefined')             // muss schon existieren !!! - also erst nach Digits / Analog aufrufen
          return _ret;

     neuroi = new Object();
     neuroi["name"] = _roi;
     _ret[_type].push(neuroi);


     return neuroi;

}

 

function CopyReferenceToImgTmp(_domainname)
{
     for (index = 0; index < 2; ++index)
     {
          _filenamevon = REFERENCES[index]["name"];
          _filenamenach = _filenamevon.replace("/config/", "/img_tmp/");
          FileDeleteOnServer(_filenamenach, _domainname);
          FileCopyOnServer(_filenamevon, _filenamenach, _domainname);
     
          _filenamevon = _filenamevon.replace(".jpg", "_org.jpg");
          _filenamenach = _filenamenach.replace(".jpg", "_org.jpg");
          FileDeleteOnServer(_filenamenach, _domainname);
          FileCopyOnServer(_filenamevon, _filenamenach, _domainname);
     }
}

function GetReferencesInfo(){
     return REFERENCES;
}


function UpdateConfigReference(_domainname){
     for (var index = 0; index < 2; ++index)
     {
          _filenamenach = REFERENCES[index]["name"];
          _filenamevon = _filenamenach.replace("/config/", "/img_tmp/");
          FileDeleteOnServer(_filenamenach, _domainname);
          FileCopyOnServer(_filenamevon, _filenamenach, _domainname);
     
          _filenamenach = _filenamenach.replace(".jpg", "_org.jpg");
          _filenamevon = _filenamevon.replace(".jpg", "_org.jpg");
          FileDeleteOnServer(_filenamenach, _domainname);
          FileCopyOnServer(_filenamevon, _filenamenach, _domainname);

     }
}


function getNUMBERInfo(){
     return NUMBERS;
}

function RenameNUMBER(_alt, _neu){
     if ((_neu.indexOf(".") >= 0) || (_neu.indexOf(",") >= 0) || 
         (_neu.indexOf(" ") >= 0) || (_neu.indexOf("\"") >= 0))
     {
          return "Number sequence name must not contain , . \" or a space";
     }

     index = -1;
     found = false;
     for (i = 0; i < NUMBERS.length; ++i) {
          if (NUMBERS[i]["name"] == _alt)
               index = i;
          if (NUMBERS[i]["name"] == _neu)
               found = true;
     }

     if (found)
          return "Number sequence name is already existing, please choose another name";

     NUMBERS[index]["name"] = _neu;
     
     return "";
}

function DeleteNUMBER(_delete){
     if (NUMBERS.length == 1)
          return "One number sequence is mandatory. Therefore this cannot be deleted"
     

     index = -1;
     for (i = 0; i < NUMBERS.length; ++i) {
          if (NUMBERS[i]["name"] == _delete)
               index = i;
     }

     if (index > -1) {
          NUMBERS.splice(index, 1);
     }

     return "";
}

function CreateNUMBER(_numbernew){
     found = false;
     for (i = 0; i < NUMBERS.length; ++i) {
          if (NUMBERS[i]["name"] == _numbernew)
               found = true;
     }

     if (found)
          return "Number sequence name is already existing, please choose another name";

     _ret = new Object();
     _ret["name"] = _numbernew;
     _ret['digit'] = new Array();
     _ret['analog'] = new Array();

     for (_cat in param)
          for (_param in param[_cat])
               if (param[_cat][_param]["Numbers"] == true)
               {
                    if (typeof (_ret[_cat]) === "undefined")
                    {
                         _ret[_cat] = new Object();
                    }
                    _ret[_cat][_param] = new Object();
                    if (param[_cat][_param]["defaultValue"] === "")
                    {
                         _ret[_cat][_param]["found"] = false;
                         _ret[_cat][_param]["enabled"] = false;
                    }
                    else
                    {
                         _ret[_cat][_param]["found"] = true;
                         _ret[_cat][_param]["enabled"] = true;
                         _ret[_cat][_param]["value1"] = param[_cat][_param]["defaultValue"];

                    }
                    _ret[_cat][_param]["anzParam"] = param[_cat][_param]["anzParam"]; 

               }

     NUMBERS.push(_ret);               
     return "";
}


function getROIInfo(_typeROI, _number){
     index = -1;
     for (var i = 0; i < NUMBERS.length; ++i)
          if (NUMBERS[i]["name"] == _number)
               index = i;

     if (index != -1)
          return NUMBERS[index][_typeROI];
     else
          return "";     
}


function RenameROI(_number, _type, _alt, _neu){
     if ((_neu.includes("=")) || (_neu.includes(".")) || (_neu.includes(":")) ||
         (_neu.includes(",")) || (_neu.includes(";")) || (_neu.includes(" ")) || 
         (_neu.includes("\""))) 
     {
          return "ROI name must not contain . : , ; = \" or space";
     }

     index = -1;
     found = false;
     _indexnumber = -1;
     for (j = 0; j < NUMBERS.length; ++j)
          if (NUMBERS[j]["name"] == _number)
               _indexnumber = j;

     if (_indexnumber == -1)
          return "Number sequence not existing. ROI cannot be renamed"  

     for (i = 0; i < NUMBERS[_indexnumber][_type].length; ++i) {
          if (NUMBERS[_indexnumber][_type][i]["name"] == _alt)
               index = i;
          if (NUMBERS[_indexnumber][_type][i]["name"] == _neu)
               found = true;
     }

     if (found)
          return "ROI name is already existing, please choose another name";

     NUMBERS[_indexnumber][_type][index]["name"] = _neu;
     
     return "";
}


function DeleteNUMBER(_delte){
     if (NUMBERS.length == 1)
          return "The last number cannot be deleted"
     
     index = -1;
     for (i = 0; i < NUMBERS.length; ++i) {
          if (NUMBERS[i]["name"] == _delte)
               index = i;
     }

     if (index > -1) {
          NUMBERS.splice(index, 1);
     }

     return "";
}


function CreateROI(_number, _type, _pos, _roinew, _x, _y, _dx, _dy, _CCW){
     _indexnumber = -1;
     for (j = 0; j < NUMBERS.length; ++j)
          if (NUMBERS[j]["name"] == _number)
               _indexnumber = j;

     if (_indexnumber == -1)
          return "Number sequence not existing. ROI cannot be created"

     found = false;
     for (i = 0; i < NUMBERS[_indexnumber][_type].length; ++i) {
          if (NUMBERS[_indexnumber][_type][i]["name"] == _roinew)
               found = true;
     }

     if (found)
          return "ROI name is already existing, please choose another name";

     _ret = new Object();
     _ret["name"] = _roinew;
     _ret["x"] = _x;
     _ret["y"] = _y;
     _ret["dx"] = _dx;
     _ret["dy"] = _dy;
     _ret["ar"] = _dx / _dy;
     _ret["CCW"] = _CCW;

     NUMBERS[_indexnumber][_type].splice(_pos+1, 0, _ret);

     return "";
}
