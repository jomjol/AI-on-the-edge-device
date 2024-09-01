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
        } 
        else {
            console.warn(request.statusText, request.responseText);
        }
    });

    try {
        url = _domainname + '/editflow?task=namenumbers';     
        xhttp.open("GET", url, false);
        xhttp.send();
    } catch (error) {}

    namenumberslist = namenumberslist.split("\t");

    return namenumberslist;
}


function getDATAList() {
    _domainname = getDomainname(); 
    datalist = "";

    var xhttp = new XMLHttpRequest();
	
    xhttp.addEventListener('load', function(event) {
        if (xhttp.status >= 200 && xhttp.status < 300) {
            datalist = xhttp.responseText;
        } 
        else {
            console.warn(request.statusText, request.responseText);
        }
    });

    try {
        url = _domainname + '/editflow?task=data';     
        xhttp.open("GET", url, false);
        xhttp.send();
    } catch (error) {}

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
        } 
        else {
            console.warn(request.statusText, request.responseText);
        }
    });

    try {
        url = _domainname + '/editflow?task=tflite';
        xhttp.open("GET", url, false);
        xhttp.send();
    } catch (error) {}

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
    ParamAddValue(param, catname, "RawImagesRetention");
    ParamAddValue(param, catname, "WaitBeforeTakingPicture");
    ParamAddValue(param, catname, "CamGainceiling");		// Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
    ParamAddValue(param, catname, "CamQuality");    		// 0 - 63
    ParamAddValue(param, catname, "CamBrightness"); 		// (-2 to 2) - set brightness
    ParamAddValue(param, catname, "CamContrast");   		//-2 - 2
    ParamAddValue(param, catname, "CamSaturation"); 		//-2 - 2
    ParamAddValue(param, catname, "CamSharpness");  		//-2 - 2
    ParamAddValue(param, catname, "CamAutoSharpness");  	// (1 or 0)	
    ParamAddValue(param, catname, "CamSpecialEffect"); 	// 0 - 6
    ParamAddValue(param, catname, "CamWbMode");        	// 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    ParamAddValue(param, catname, "CamAwb");           	// white balance enable (0 or 1)
    ParamAddValue(param, catname, "CamAwbGain");       	// Auto White Balance enable (0 or 1)
    ParamAddValue(param, catname, "CamAec");           	// auto exposure off (1 or 0)
    ParamAddValue(param, catname, "CamAec2");          	// automatic exposure sensor  (0 or 1)
    ParamAddValue(param, catname, "CamAeLevel");       	// auto exposure levels (-2 to 2)
    ParamAddValue(param, catname, "CamAecValue");      	// set exposure manually  (0-1200)
    ParamAddValue(param, catname, "CamAgc");           	// auto gain off (1 or 0)
    ParamAddValue(param, catname, "CamAgcGain");       	// set gain manually (0 - 30)
    ParamAddValue(param, catname, "CamBpc");           	// black pixel correction
    ParamAddValue(param, catname, "CamWpc");           	// white pixel correction
    ParamAddValue(param, catname, "CamRawGma");        	// (1 or 0)
    ParamAddValue(param, catname, "CamLenc");          	// lens correction (1 or 0)
    ParamAddValue(param, catname, "CamHmirror");       	// (0 or 1) flip horizontally
    ParamAddValue(param, catname, "CamVflip");         	// Invert image (0 or 1)
    ParamAddValue(param, catname, "CamDcw");           	// downsize enable (1 or 0)
    ParamAddValue(param, catname, "CamDenoise");        // The OV2640 does not support it, OV3660 and OV5640 (0 to 8)
    ParamAddValue(param, catname, "CamZoom");
    ParamAddValue(param, catname, "CamZoomOffsetX");
    ParamAddValue(param, catname, "CamZoomOffsetY");
    ParamAddValue(param, catname, "CamZoomSize");
    ParamAddValue(param, catname, "LEDIntensity");
    ParamAddValue(param, catname, "Demo");

    var catname = "Alignment";
    category[catname] = new Object();
    category[catname]["enabled"] = false;
    category[catname]["found"] = false;
    param[catname] = new Object();
    ParamAddValue(param, catname, "InitialRotate");
    ParamAddValue(param, catname, "SearchFieldX");
    ParamAddValue(param, catname, "SearchFieldY");
    ParamAddValue(param, catname, "AlignmentAlgo");

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
    ParamAddValue(param, catname, "AnalogDigitalTransitionStart", 1, true, "9.2");
    ParamAddValue(param, catname, "ChangeRateThreshold", 1, true, "2");
    // ParamAddValue(param, catname, "PreValueUse", 1, true, "true");
    ParamAddValue(param, catname, "PreValueUse");
    ParamAddValue(param, catname, "PreValueAgeStartup");
    ParamAddValue(param, catname, "AllowNegativeRates", 1, true, "false");
    ParamAddValue(param, catname, "MaxRateValue", 1, true, "0.05");
    ParamAddValue(param, catname, "MaxRateType", 1, true);
    ParamAddValue(param, catname, "ExtendedResolution", 1, true, "false");
    ParamAddValue(param, catname, "IgnoreLeadingNaN", 1, true, "false");
    // ParamAddValue(param, catname, "IgnoreAllNaN", 1, true, "false");
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
    ParamAddValue(param, catname, "Bucket");
//     ParamAddValue(param, catname, "Measurement");
    ParamAddValue(param, catname, "Org");
    ParamAddValue(param, catname, "Token");
    ParamAddValue(param, catname, "Measurement", 1, true);
    ParamAddValue(param, catname, "Field", 1, true);

    var catname = "Webhook";
    category[catname] = new Object();
    category[catname]["enabled"] = false;
    category[catname]["found"] = false;
    param[catname] = new Object();
    ParamAddValue(param, catname, "Uri");
    ParamAddValue(param, catname, "ApiKey");
    ParamAddValue(param, catname, "UploadImg");

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
    ParamAddValue(param, catname, "Tooltip");	
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
    if (category["DataLogging"]["found"] == false) {
        category["DataLogging"]["found"] = true;
        category["DataLogging"]["enabled"] = true;

        param["DataLogging"]["DataLogActive"]["found"] = true;
        param["DataLogging"]["DataLogActive"]["enabled"] = true;
        param["DataLogging"]["DataLogActive"]["value1"] = "true";
          
        param["DataLogging"]["DataFilesRetention"]["found"] = true;
        param["DataLogging"]["DataFilesRetention"]["enabled"] = true;
        param["DataLogging"]["DataFilesRetention"]["value1"] = "3";
    }

    if (category["DataLogging"]["enabled"] == false) {
        category["DataLogging"]["enabled"] = true
    }

    if (param["DataLogging"]["DataLogActive"]["enabled"] == false && param["DataLogging"]["DataLogActive"]["value1"] == "") {
        param["DataLogging"]["DataLogActive"]["found"] = true;
        param["DataLogging"]["DataLogActive"]["enabled"] = true;
        param["DataLogging"]["DataLogActive"]["value1"] = "true";
    }

    if (param["DataLogging"]["DataFilesRetention"]["enabled"] == false && param["DataLogging"]["DataFilesRetention"]["value1"] == "") {
        param["DataLogging"]["DataFilesRetention"]["found"] = true;
        param["DataLogging"]["DataFilesRetention"]["enabled"] = true;
        param["DataLogging"]["DataFilesRetention"]["value1"] = "3";
    }

    // Downward compatibility: Create RSSIThreshold if not available
    if (param["System"]["RSSIThreshold"]["found"] == false) {
        param["System"]["RSSIThreshold"]["found"] = true;
        param["System"]["RSSIThreshold"]["enabled"] = false;
        param["System"]["RSSIThreshold"]["value1"] = "0";
    }
}


function ParamAddValue(param, _cat, _param, _anzParam = 1, _isNUMBER = false, _defaultValue = "", _checkRegExList = null) {
    param[_cat][_param] = new Object(); 
    param[_cat][_param]["found"] = false;
    param[_cat][_param]["enabled"] = false;
    param[_cat][_param]["line"] = -1; 
    param[_cat][_param]["anzParam"] = _anzParam;
    param[_cat][_param]["defaultValue"] = _defaultValue;
    param[_cat][_param]["Numbers"] = _isNUMBER;
    param[_cat][_param].checkRegExList = _checkRegExList;
};


function ParseConfigParamAll(_aktline, _catname) {
    ++_aktline;

    while ((_aktline < config_split.length) && !(config_split[_aktline][0] == "[") && !((config_split[_aktline][0] == ";") && (config_split[_aktline][1] == "["))) {
        var _input = config_split[_aktline];
        let [isCom, input] = isCommented(_input);
        var linesplit = ZerlegeZeile(input);
        ParamExtractValueAll(param, linesplit, _catname, _aktline, isCom);
        
        if (!isCom && (linesplit.length >= 5) && (_catname == 'Digits')) {
            ExtractROIs(input, "digit");
        }
        
        if (!isCom && (linesplit.length >= 5) && (_catname == 'Analog')) {
            ExtractROIs(input, "analog");
        }
        
        if (!isCom && (linesplit.length == 3) && (_catname == 'Alignment')) {
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


function ParamExtractValue(_param, _linesplit, _catname, _paramname, _aktline, _iscom, _anzvalue = 1) {
    if ((_linesplit[0].toUpperCase() == _paramname.toUpperCase()) && (_linesplit.length > _anzvalue)) {
        _param[_catname][_paramname]["found"] = true;
        _param[_catname][_paramname]["enabled"] = !_iscom;
        _param[_catname][_paramname]["line"] = _aktline;
        _param[_catname][_paramname]["anzpara"] = _anzvalue;
        
        for (var j = 1; j <= _anzvalue; ++j) {
            _param[_catname][_paramname]["value"+j] = _linesplit[j];
        }
    }
}


function ParamExtractValueAll(_param, _linesplit, _catname, _aktline, _iscom) {
    for (var paramname in _param[_catname]) {
        _AktROI = "default";
        _AktPara = _linesplit[0];
        _pospunkt = _AktPara.indexOf (".");
        
        if (_pospunkt > -1) {
            _AktROI = _AktPara.substring(0, _pospunkt);
            _AktPara = _AktPara.substring(_pospunkt+1);
        }
        
        if (_AktPara.toUpperCase() == paramname.toUpperCase()) {
            while (_linesplit.length <= _param[_catname][paramname]["anzParam"]) {
                _linesplit.push("");
            }

            _param[_catname][paramname]["found"] = true;
            _param[_catname][paramname]["enabled"] = !_iscom;
            _param[_catname][paramname]["line"] = _aktline;
            
            if (_param[_catname][paramname]["Numbers"] == true) {        // möglicher Multiusage
                abc = getNUMBERS(_linesplit[0]);
                abc[_catname][paramname] = new Object;
                abc[_catname][paramname]["found"] = true;
                abc[_catname][paramname]["enabled"] = !_iscom;
     
                for (var j = 1; j <= _param[_catname][paramname]["anzParam"]; ++j) {
                    abc[_catname][paramname]["value"+j] = _linesplit[j];
                }
				
                if (abc["name"] == "default") {
                    for (_num in NUMBERS) {        // wert mit Default belegen
                        if (NUMBERS[_num][_catname][paramname]["found"] == false) {
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
            else {
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


function getCamConfig() {			
    ParseConfig();		

    param["System"]["Tooltip"]["enabled"] = true;
    param["Alignment"]["InitialRotate"]["enabled"] = true;
			
    param["TakeImage"]["WaitBeforeTakingPicture"]["enabled"] = true;
    param["TakeImage"]["CamGainceiling"]["enabled"] = true;		// Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
    param["TakeImage"]["CamQuality"]["enabled"] = true;    		// 0 - 63
    param["TakeImage"]["CamBrightness"]["enabled"] = true; 		// (-2 to 2) - set brightness
    param["TakeImage"]["CamContrast"]["enabled"] = true;   		//-2 - 2
    param["TakeImage"]["CamSaturation"]["enabled"] = true; 		//-2 - 2
    param["TakeImage"]["CamSharpness"]["enabled"] = true;  		//-2 - 2
    param["TakeImage"]["CamAutoSharpness"]["enabled"] = true;  	//(1 or 0)
    param["TakeImage"]["CamSpecialEffect"]["enabled"] = true;	// 0 - 6
    param["TakeImage"]["CamWbMode"]["enabled"] = true;        	// 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    param["TakeImage"]["CamAwb"]["enabled"] = true;           	// white balance enable (0 or 1)
    param["TakeImage"]["CamAwbGain"]["enabled"] = true;       	// Auto White Balance enable (0 or 1)
    param["TakeImage"]["CamAec"]["enabled"] = true;           	// auto exposure off (1 or 0)
    param["TakeImage"]["CamAec2"]["enabled"] = true;          	// automatic exposure sensor  (0 or 1)
    param["TakeImage"]["CamAeLevel"]["enabled"] = true;       	// auto exposure levels (-2 to 2)
    param["TakeImage"]["CamAecValue"]["enabled"] = true;      	// set exposure manually  (0-1200)
    param["TakeImage"]["CamAgc"]["enabled"] = true;           	// auto gain off (1 or 0)
    param["TakeImage"]["CamAgcGain"]["enabled"] = true;       	// set gain manually (0 - 30)
    param["TakeImage"]["CamBpc"]["enabled"] = true;          	// black pixel correction
    param["TakeImage"]["CamWpc"]["enabled"] = true;           	// white pixel correction
    param["TakeImage"]["CamRawGma"]["enabled"] = true;        	// (1 or 0)
    param["TakeImage"]["CamLenc"]["enabled"] = true;          	// lens correction (1 or 0)
    param["TakeImage"]["CamHmirror"]["enabled"] = true;       	// (0 or 1) flip horizontally
    param["TakeImage"]["CamVflip"]["enabled"] = true;         	// Invert image (0 or 1)
    param["TakeImage"]["CamDcw"]["enabled"] = true;           	// downsize enable (1 or 0)
    param["TakeImage"]["CamDenoise"]["enabled"] = true;       	// The OV2640 does not support it, OV3660 and OV5640 (0 to 8)
    param["TakeImage"]["CamZoom"]["enabled"] = true;
    param["TakeImage"]["CamZoomOffsetX"]["enabled"] = true;
    param["TakeImage"]["CamZoomOffsetY"]["enabled"] = true;
    param["TakeImage"]["CamZoomSize"]["enabled"] = true;
    param["TakeImage"]["LEDIntensity"]["enabled"] = true;

    if (!param["System"]["Tooltip"]["found"]) {
        param["System"]["Tooltip"]["found"] = true;
        param["System"]["Tooltip"].value1 = 'true';
    }

    if (!param["Alignment"]["InitialRotate"]["found"]) {
        param["Alignment"]["InitialRotate"]["found"] = true;
        param["Alignment"]["InitialRotate"].value1 = 'false';
    }

    if (!param["TakeImage"]["WaitBeforeTakingPicture"]["found"]) {
        param["TakeImage"]["WaitBeforeTakingPicture"]["found"] = true;
        param["TakeImage"]["WaitBeforeTakingPicture"].value1 = '5';
    }
    if (!param["TakeImage"]["CamGainceiling"]["found"]) {
        param["TakeImage"]["CamGainceiling"]["found"] = true;
        // param["TakeImage"]["CamGainceiling"].value1 = '2';
        param["TakeImage"]["CamGainceiling"].value1 = 'x8';
    }
    if (!param["TakeImage"]["CamQuality"]["found"]) {
        param["TakeImage"]["CamQuality"]["found"] = true;
        param["TakeImage"]["CamQuality"].value1 = '10';
    }
    if (!param["TakeImage"]["CamBrightness"]["found"]) {
        param["TakeImage"]["CamBrightness"]["found"] = true;
        param["TakeImage"]["CamBrightness"].value1 = '0';
    }
    if (!param["TakeImage"]["CamContrast"]["found"]) {
        param["TakeImage"]["CamContrast"]["found"] = true;
        param["TakeImage"]["CamContrast"].value1 = '0';
    }
    if (!param["TakeImage"]["CamSaturation"]["found"]) {
        param["TakeImage"]["CamSaturation"]["found"] = true;
        param["TakeImage"]["CamSaturation"].value1 = '0';
    }
    if (!param["TakeImage"]["CamSharpness"]["found"]) {
        param["TakeImage"]["CamSharpness"]["found"] = true;
        param["TakeImage"]["CamSharpness"].value1 = '0';
    }
    if (!param["TakeImage"]["CamAutoSharpness"]["found"]) {
        param["TakeImage"]["CamAutoSharpness"]["found"] = true;
        param["TakeImage"]["CamAutoSharpness"].value1 = 'false';
    }			
    if (!param["TakeImage"]["CamSpecialEffect"]["found"]) {
        param["TakeImage"]["CamSpecialEffect"]["found"] = true;
        param["TakeImage"]["CamSpecialEffect"].value1 = 'no_effect';
    }		
    if (!param["TakeImage"]["CamWbMode"]["found"]) {
        param["TakeImage"]["CamWbMode"]["found"] = true;
        param["TakeImage"]["CamWbMode"].value1 = 'auto';
    }			
    if (!param["TakeImage"]["CamAwb"]["found"]) {
        param["TakeImage"]["CamAwb"]["found"] = true;
        param["TakeImage"]["CamAwb"].value1 = 'true';
    }
    if (!param["TakeImage"]["CamAwbGain"]["found"]) {
        param["TakeImage"]["CamAwbGain"]["found"] = true;
        param["TakeImage"]["CamAwbGain"].value1 = 'true';
    }
    if (!param["TakeImage"]["CamAec"]["found"]) {
        param["TakeImage"]["CamAec"]["found"] = true;
        param["TakeImage"]["CamAec"].value1 = 'true';
    }
    if (!param["TakeImage"]["CamAec2"]["found"]) {
        param["TakeImage"]["CamAec2"]["found"] = true;
        param["TakeImage"]["CamAec2"].value1 = 'true';
    }
    if (!param["TakeImage"]["CamAeLevel"]["found"]) {
        param["TakeImage"]["CamAeLevel"]["found"] = true;
        param["TakeImage"]["CamAeLevel"].value1 = '2';
    }
    if (!param["TakeImage"]["CamAecValue"]["found"]) {
        param["TakeImage"]["CamAecValue"]["found"] = true;
        param["TakeImage"]["CamAecValue"].value1 = '600';
    }
    if (!param["TakeImage"]["CamAgc"]["found"]) {
        param["TakeImage"]["CamAgc"]["found"] = true;
        param["TakeImage"]["CamAgc"].value1 = 'true';
    }
    if (!param["TakeImage"]["CamAgcGain"]["found"]) {
        param["TakeImage"]["CamAgcGain"]["found"] = true;
        param["TakeImage"]["CamAgcGain"].value1 = '8';
    }
    if (!param["TakeImage"]["CamBpc"]["found"]) {
        param["TakeImage"]["CamBpc"]["found"] = true;
        param["TakeImage"]["CamBpc"].value1 = 'true';
    }
    if (!param["TakeImage"]["CamWpc"]["found"]) {
        param["TakeImage"]["CamWpc"]["found"] = true;
        param["TakeImage"]["CamWpc"].value1 = 'true';
    }		
    if (!param["TakeImage"]["CamRawGma"]["found"]) {
        param["TakeImage"]["CamRawGma"]["found"] = true;
        param["TakeImage"]["CamRawGma"].value1 = 'true';
    }		
    if (!param["TakeImage"]["CamLenc"]["found"]) {
        param["TakeImage"]["CamLenc"]["found"] = true;
        param["TakeImage"]["CamLenc"].value1 = 'true';
    }			
    if (!param["TakeImage"]["CamHmirror"]["found"]) {
        param["TakeImage"]["CamHmirror"]["found"] = true;
        param["TakeImage"]["CamHmirror"].value1 = 'false';
    }
    if (!param["TakeImage"]["CamVflip"]["found"]) {
        param["TakeImage"]["CamVflip"]["found"] = true;
        param["TakeImage"]["CamVflip"].value1 = 'false';
    }
    if (!param["TakeImage"]["CamDcw"]["found"]) {
        param["TakeImage"]["CamDcw"]["found"] = true;
        param["TakeImage"]["CamDcw"].value1 = 'true';
    }
    if (!param["TakeImage"]["CamDenoise"]["found"]) {
        param["TakeImage"]["CamDenoise"]["found"] = true;
        param["TakeImage"]["CamDenoise"].value1 = '0';
    }
    if (!param["TakeImage"]["CamZoom"]["found"]) {
        param["TakeImage"]["CamZoom"]["found"] = true;
        param["TakeImage"]["CamZoom"].value1 = 'false';
    }
    if (!param["TakeImage"]["CamZoomOffsetX"]["found"]) {
        param["TakeImage"]["CamZoomOffsetX"]["found"] = true;
        param["TakeImage"]["CamZoomOffsetX"].value1 = '0';
    }
    if (!param["TakeImage"]["CamZoomOffsetY"]["found"]) {
        param["TakeImage"]["CamZoomOffsetY"]["found"] = true;
        param["TakeImage"]["CamZoomOffsetY"].value1 = '0';
    }
    if (!param["TakeImage"]["CamZoomSize"]["found"]) {
        param["TakeImage"]["CamZoomSize"]["found"] = true;
        param["TakeImage"]["CamZoomSize"].value1 = '0';
    }
    if (!param["TakeImage"]["LEDIntensity"]["found"]) {
        param["TakeImage"]["LEDIntensity"]["found"] = true;
        param["TakeImage"]["LEDIntensity"].value1 = '50';
    }

    return param;	
}


function getConfigParameters() {
    return param;
}


function WriteConfigININew() {
    // Cleanup empty NUMBERS
    for (var j = 0; j < NUMBERS.length; ++j) {
        if ((NUMBERS[j]["digit"].length + NUMBERS[j]["analog"].length) == 0) {
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
            if (param[cat][name]["Numbers"]) {
                for (_num in NUMBERS) {
                    text = NUMBERS[_num]["name"] + "." + name;

                    var text = text + " =" 
                         
                    for (var j = 1; j <= param[cat][name]["anzParam"]; ++j) {
                        if (!(typeof NUMBERS[_num][cat][name]["value"+j] == 'undefined')) {
                            text = text + " " + NUMBERS[_num][cat][name]["value"+j];
                        }
                    }
						 
                    if (!NUMBERS[_num][cat][name]["enabled"]) {
                        text = ";" + text;
                    }
						 
                    config_split.push(text);
                }
            }
            else {
                var text = name + " =" 
                    
                for (var j = 1; j <= param[cat][name]["anzParam"]; ++j) {
                    if (!(typeof param[cat][name]["value"+j] == 'undefined')) {
                        text = text + " " + param[cat][name]["value"+j];
                    }
                }
					
                if (!param[cat][name]["enabled"]) {
                    text = ";" + text;
                }
					
                config_split.push(text);
            }
        }
		  
        if (cat == "Digits") {
            for (var _roi in NUMBERS) {
                if (NUMBERS[_roi]["digit"].length > 0) {
                    for (var _roiddet in NUMBERS[_roi]["digit"]) {
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
		  
        if (cat == "Analog") {
            for (var _roi in NUMBERS) {
                if (NUMBERS[_roi]["analog"].length > 0) {
                    for (var _roiddet in NUMBERS[_roi]["analog"]) {
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
		  
        if (cat == "Alignment") {
            for (var _roi in REFERENCES) {
                text = REFERENCES[_roi]["name"];
                text = text + " " + REFERENCES[_roi]["x"];
                text = text + " " + REFERENCES[_roi]["y"];
                config_split.push(text);
            }
        }

        config_split.push("");
    }
}


function isCommented(input) {
    let isComment = false;
		  
    if (input.charAt(0) == ';') {
        isComment = true;
        input = input.substr(1, input.length-1);
    }
		  
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
    
    if (linesplit.length >= 6) {
        abc["CCW"] = linesplit[5];
	}
}


function getNUMBERS(_name, _type, _create = true) {
    _pospunkt = _name.indexOf (".");
    
    if (_pospunkt > -1) {
        _digit = _name.substring(0, _pospunkt);
        _roi = _name.substring(_pospunkt+1);
    }
    else {
        _digit = "default";
        _roi = _name;
    }

    _ret = -1;

    for (i = 0; i < NUMBERS.length; ++i) {
        if (NUMBERS[i]["name"] == _digit) {
            _ret = NUMBERS[i];
        }
    }

    if (!_create) {         // nicht gefunden und soll auch nicht erzeugt werden, ggf. geht eine NULL zurück
          return _ret;
    }

    if (_ret == -1) {
        _ret = new Object();
        _ret["name"] = _digit;
        _ret['digit'] = new Array();
        _ret['analog'] = new Array();

        for (_cat in param) {
            for (_param in param[_cat]) {
                if (param[_cat][_param]["Numbers"] == true){
                    if (typeof  _ret[_cat] == 'undefined') {
                        _ret[_cat] = new Object();
                    }
					
                    _ret[_cat][_param] = new Object();
                    _ret[_cat][_param]["found"] = false;
                    _ret[_cat][_param]["enabled"] = false;
                    _ret[_cat][_param]["anzParam"] = param[_cat][_param]["anzParam"]; 
                }
            }
        }

        NUMBERS.push(_ret);
    }

    if (typeof _type == 'undefined') {            // muss schon existieren !!! - also erst nach Digits / Analog aufrufen
        return _ret;
    }

    neuroi = new Object();
    neuroi["name"] = _roi;
    _ret[_type].push(neuroi);

    return neuroi;
}

 
function CopyReferenceToImgTmp(_domainname) {
    for (index = 0; index < 2; ++index) {
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


function UpdateConfigReferences(_domainname){
    for (var index = 0; index < 2; ++index) {
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


function UpdateConfigReference(_anzneueref, _domainname){
    var index = 0;

    if (_anzneueref == 1) {	
        index = 0;
    }

    else if (_anzneueref == 2) {
        index = 1;
    }

    _filenamenach = REFERENCES[index]["name"];
    _filenamevon = _filenamenach.replace("/config/", "/img_tmp/");

    FileDeleteOnServer(_filenamenach, _domainname);
    FileCopyOnServer(_filenamevon, _filenamenach, _domainname);

    _filenamenach = _filenamenach.replace(".jpg", "_org.jpg");
    _filenamevon = _filenamevon.replace(".jpg", "_org.jpg");

    FileDeleteOnServer(_filenamenach, _domainname);
    FileCopyOnServer(_filenamevon, _filenamenach, _domainname);
}	


function getNUMBERInfo(){
     return NUMBERS;
}


function RenameNUMBER(_alt, _neu){
    if ((_neu.indexOf(".") >= 0) || (_neu.indexOf(",") >= 0) || (_neu.indexOf(" ") >= 0) || (_neu.indexOf("\"") >= 0)) {
        return "Number sequence name must not contain , . \" or a space";
    }

    index = -1;
    found = false;
    
	for (i = 0; i < NUMBERS.length; ++i) {
        if (NUMBERS[i]["name"] == _alt) {
            index = i;
        }
		
        if (NUMBERS[i]["name"] == _neu) {
            found = true;
        }
    }

    if (found) {
        return "Number sequence name is already existing, please choose another name";
    }

    NUMBERS[index]["name"] = _neu;
     
    return "";
}


function DeleteNUMBER(_delete){
    if (NUMBERS.length == 1) {
        return "One number sequence is mandatory. Therefore this cannot be deleted"
    }
     
    index = -1;
	 
    for (i = 0; i < NUMBERS.length; ++i) {
        if (NUMBERS[i]["name"] == _delete) {
            index = i;
        }
    }

    if (index > -1) {
        NUMBERS.splice(index, 1);
    }

    return "";
}


function CreateNUMBER(_numbernew){
    found = false;
    
    for (i = 0; i < NUMBERS.length; ++i) {
        if (NUMBERS[i]["name"] == _numbernew) {
            found = true;
        }
    }

    if (found) {
        return "Number sequence name is already existing, please choose another name";
    }

    _ret = new Object();
    _ret["name"] = _numbernew;
    _ret['digit'] = new Array();
    _ret['analog'] = new Array();

    for (_cat in param) {
        for (_param in param[_cat]) {
            if (param[_cat][_param]["Numbers"] == true) {
                if (typeof (_ret[_cat]) === "undefined") {
                    _ret[_cat] = new Object();
                }
					
                _ret[_cat][_param] = new Object();
					
                if (param[_cat][_param]["defaultValue"] === "") {
                    _ret[_cat][_param]["found"] = false;
                    _ret[_cat][_param]["enabled"] = false;
                }
                else {
                    _ret[_cat][_param]["found"] = true;
                    _ret[_cat][_param]["enabled"] = true;
                    _ret[_cat][_param]["value1"] = param[_cat][_param]["defaultValue"];

                }
					
                _ret[_cat][_param]["anzParam"] = param[_cat][_param]["anzParam"]; 
            }
        }
    }

    NUMBERS.push(_ret);               
    return "";
}


function getROIInfo(_typeROI, _number){
    index = -1;
    
    for (var i = 0; i < NUMBERS.length; ++i) {
        if (NUMBERS[i]["name"] == _number) {
            index = i;
        }
    }

    if (index != -1) {
        return NUMBERS[index][_typeROI];
    }
    else {
        return "";
    }
}


function RenameROI(_number, _type, _alt, _neu){
    if ((_neu.includes("=")) || (_neu.includes(".")) || (_neu.includes(":")) || (_neu.includes(",")) || (_neu.includes(";")) || (_neu.includes(" ")) || (_neu.includes("\""))) {
        return "ROI name must not contain . : , ; = \" or space";
    }

    index = -1;
    found = false;
    _indexnumber = -1;
    
    for (j = 0; j < NUMBERS.length; ++j) {
        if (NUMBERS[j]["name"] == _number) {
            _indexnumber = j;
        }
    }

    if (_indexnumber == -1) {
        return "Number sequence not existing. ROI cannot be renamed"
    }

    for (i = 0; i < NUMBERS[_indexnumber][_type].length; ++i) {
        if (NUMBERS[_indexnumber][_type][i]["name"] == _alt) {
            index = i;
        }
		
        if (NUMBERS[_indexnumber][_type][i]["name"] == _neu) {
            found = true;
        }
    }

    if (found) {
        return "ROI name is already existing, please choose another name";
    }

    NUMBERS[_indexnumber][_type][index]["name"] = _neu;
     
    return "";
}


function DeleteNUMBER(_delte) {
    if (NUMBERS.length == 1) {
        return "The last number cannot be deleted"
    }
	
    index = -1;
    
    for (i = 0; i < NUMBERS.length; ++i) {
        if (NUMBERS[i]["name"] == _delte) {
            index = i;
        }
    }

    if (index > -1) {
        NUMBERS.splice(index, 1);
    }

    return "";
}


function CreateROI(_number, _type, _pos, _roinew, _x, _y, _dx, _dy, _CCW){
    _indexnumber = -1;
    
    for (j = 0; j < NUMBERS.length; ++j) {
        if (NUMBERS[j]["name"] == _number) {
            _indexnumber = j;
        }
    }

    if (_indexnumber == -1) {
        return "Number sequence not existing. ROI cannot be created"
    }

    found = false;
    
    for (i = 0; i < NUMBERS[_indexnumber][_type].length; ++i) {
        if (NUMBERS[_indexnumber][_type][i]["name"] == _roinew) {
            found = true;
        }
    }

    if (found) {
        return "ROI name is already existing, please choose another name";
    }

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
