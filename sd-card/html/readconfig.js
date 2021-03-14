function readconfig_Version(){
     return "1.0.0 - 20200910";
 }

var config_gesamt;
var config_split;
var ref = new Array(2);
var digit = new Array(0);
var analog = new Array(0);
var initalrotate = new Object();
var analogEnabled = false;
var posAnalogHeader;
var digitsEnabled = false;
var posDigitsHeader;

function MakeRefZW(zw, _basepath){
     _filetarget = zw["name"].replace("/config/", "/img_tmp/");
     _filetarget = _filetarget.replace(".jpg", "_org.jpg");
     url = _basepath + "/editflow.html?task=cutref&in=/config/reference.jpg&out="+_filetarget+"&x=" + zw["x"] + "&y="  + zw["y"] + "&dx=" + zw["dx"] + "&dy=" + zw["dy"];
     var xhttp = new XMLHttpRequest();  
     try {
          xhttp.open("GET", url, false);
          xhttp.send();     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }
     _filetarget2 = zw["name"].replace("/config/", "/img_tmp/");
//     _filetarget2 = _filetarget2.replace(".jpg", "_org.jpg");
     FileCopyOnServer(_filetarget, _filetarget2, _basepath);
}

function CopyReferenceToImgTmp(_basepath)
{
     for (index = 0; index < 2; ++index)
     {
          _filenamevon = ref[index]["name"];
          _filenamenach = _filenamevon.replace("/config/", "/img_tmp/");
          FileDeleteOnServer(_filenamenach, _basepath);
          FileCopyOnServer(_filenamevon, _filenamenach, _basepath);
     
          _filenamevon = _filenamevon.replace(".jpg", "_org.jpg");
          _filenamenach = _filenamenach.replace(".jpg", "_org.jpg");
          FileDeleteOnServer(_filenamenach, _basepath);
          FileCopyOnServer(_filenamevon, _filenamenach, _basepath);
     }
}

function GetReferencesInfo(){
     return ref;
}

function ParseConfigAlignment(_aktline){
     var akt_ref = 0;
     ++_aktline;

     while ((_aktline < config_split.length) 
            && !(config_split[_aktline][0] == "[") 
            && !((config_split[_aktline][0] == ";") && (config_split[_aktline][1] == "["))) {
          var linesplit = ZerlegeZeile(config_split[_aktline]);
          if ((linesplit[0].toUpperCase() == "INITIALMIRROR") && (linesplit.length > 1))
          {
              initalrotate["mirror"] = linesplit[1].toUpperCase().localeCompare("TRUE") == 0;
              initalrotate["pos_config_mirror"] = _aktline;
          }          

          if (((linesplit[0].toUpperCase() == "INITALROTATE") || (linesplit[0].toUpperCase() == "INITIALROTATE"))  && (linesplit.length > 1))
          {
              initalrotate["angle"] = parseInt(linesplit[1]);
              initalrotate["pos_config"] = _aktline;
          }          
          if (linesplit.length == 3)
          {
               ref[akt_ref] = new Object();
               ref[akt_ref]["pos_ref"] = _aktline;
               ref[akt_ref]["name"] = linesplit[0];
               ref[akt_ref]["x"] = linesplit[1];
               ref[akt_ref]["y"] = linesplit[2];
               akt_ref++;
          }
          ++_aktline;
     }    
     return _aktline; 
}

function ParseConfigDigit(_aktline){
     ++_aktline;
     digit.length = 0;

     while ((_aktline < config_split.length) 
            && !(config_split[_aktline][0] == "[") 
            && !((config_split[_aktline][0] == ";") && (config_split[_aktline][1] == "["))) {
          var linesplit = ZerlegeZeile(config_split[_aktline]);
          if (linesplit.length >= 5)
          {
               zw = new Object();
               zw["pos_ref"] = _aktline;
               zw["name"] = linesplit[0];
               zw["x"] = linesplit[1];
               zw["y"] = linesplit[2];
               zw["dx"] = linesplit[3];
               zw["dy"] = linesplit[4];
               zw["ar"] = parseFloat(linesplit[3]) / parseFloat(linesplit[4]);
               digit.push(zw);
          }
          ++_aktline;
     }    
     return _aktline; 
}

function GetAnalogEnabled() {
     return analogEnabled;
}


function GetDigitsEnabled() {
     return digitsEnabled;
}


function ParseConfigAnalog(_aktline){
     ++_aktline;
     analog.length = 0;

     while ((_aktline < config_split.length) 
            && !(config_split[_aktline][0] == "[") 
            && !((config_split[_aktline][0] == ";") && (config_split[_aktline][1] == "["))) {
          var linesplit = ZerlegeZeile(config_split[_aktline]);
          if (linesplit.length >= 5)
          {
               zw = new Object();
               zw["pos_ref"] = _aktline;
               zw["name"] = linesplit[0];
               zw["x"] = linesplit[1];
               zw["y"] = linesplit[2];
               zw["dx"] = linesplit[3];
               zw["dy"] = linesplit[4];
               zw["ar"] = parseFloat(linesplit[3]) / parseFloat(linesplit[4]);
               analog.push(zw);
          }
          ++_aktline;
     }    
     return _aktline; 
}


function getROIInfo(_typeROI){
     if (_typeROI == "[Digits]"){
          targetROI = digit;
     }
     if (_typeROI == "[Analog]"){
          targetROI = analog;
     }
     return targetROI.slice();         // Kopie senden, nicht orginal!!!
}

function SaveROIToConfig(_ROIInfo, _typeROI, _basepath, _enabled){
     if (_enabled) {
          text = _typeROI;
     }
     else {
          text = ";" + _typeROI;
     }

     if (_typeROI == "[Digits]"){
          config_split[posDigitsHeader] = text;
          targetROI = digit;
     }

     if (_typeROI == "[Analog]"){
          config_split[posAnalogHeader] = text;
          targetROI = analog;
     }

     // Abstimmen Anzahl ROIs:
     var _pos = targetROI[targetROI.length-1]["pos_ref"];

     for (var i = targetROI.length; i < _ROIInfo.length; ++i){
          var zw = config_split[config_split.length-1];
          config_split.push(zw);
          for (var j = config_split.length-2; j > _pos + 1; --j){
               config_split[j] = config_split[j-1];
          }
     }

     for (i = targetROI.length-1; i > _ROIInfo.length-1; --i){
          var _zwpos = targetROI[i]["pos_ref"];
          config_split.splice(_zwpos, 1);
     }

     var linewrite = 0;
     for (i = 0; i < _ROIInfo.length; ++i){
          if (i < targetROI.length){
               linewrite = targetROI[i]["pos_ref"];
          }
          else {
               linewrite++;
          }
          config_split[linewrite] = _ROIInfo[i]["name"] + " " + _ROIInfo[i]["x"] + " " + _ROIInfo[i]["y"] + " " + _ROIInfo[i]["dx"] + " " + _ROIInfo[i]["dy"];
     }

     SaveConfigToServer(_basepath);
}


function ParseConfig() {
     config_split = config_gesamt.split("\n");
     var aktline = 0;

     while (aktline < config_split.length){
          if ((config_split[aktline].trim().toUpperCase() == "[ALIGNMENT]") || (config_split[aktline].trim().toUpperCase() == ";[ALIGNMENT]")){
               aktline = ParseConfigAlignment(aktline);
               continue;
          }
          if ((config_split[aktline].trim().toUpperCase() == "[DIGITS]") || (config_split[aktline].trim().toUpperCase() == ";[DIGITS]")){
               posDigitsHeader = aktline;
               if (config_split[aktline][0] == "[") {
                    digitsEnabled = true;
               }
               aktline = ParseConfigDigit(aktline);
               continue;
          }

          if ((config_split[aktline].trim().toUpperCase() == "[ANALOG]") || (config_split[aktline].trim().toUpperCase() == ";[ANALOG]")) {
               posAnalogHeader = aktline;
               if (config_split[aktline][0] == "[") {
                    analogEnabled = true;
               }
               aktline = ParseConfigAnalog(aktline);
               continue;
          }

          aktline++;
     }
}

function getPreRotate(){
     return initalrotate["angle"];
}

function setPreRotate(_prerotate){
     initalrotate["angle"] = _prerotate;
}

function getMirror(){
     if (initalrotate.hasOwnProperty("mirror")) {
          return initalrotate["mirror"];
     }
     return false;
}

function setMirror(_mirror){
     initalrotate["mirror"] = _mirror;
}

function SaveCanvasToImage(_canvas, _filename, _delete = true, _basepath = ""){
     var JPEG_QUALITY=0.8;
     var dataUrl = _canvas.toDataURL('image/jpeg', JPEG_QUALITY);	
     var rtn = dataURLtoBlob(dataUrl);

     if (_delete) {
          FileDeleteOnServer(_filename, _basepath);
     }
	
     FileSendContent(rtn, _filename, _basepath);
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

function UpdateConfigFileReferenceChange(_basepath){
     for (var _index = 0; _index < ref.length; ++_index){
          var zeile = ref[_index]["name"] + " " + ref[_index]["x"] + " " + ref[_index]["y"];
          var _pos = ref[_index]["pos_ref"];
          config_split[_pos] = zeile;          
     }

     zeile = "InitialRotate = " + initalrotate["angle"];
     var _pos = initalrotate["pos_config"];
     config_split[_pos] = zeile;

     var mirror = false;
     if (initalrotate.hasOwnProperty("mirror")) {
          mirror = initalrotate["mirror"];
     }
     var mirror_pos = -1;
     if (initalrotate.hasOwnProperty("pos_config_mirror")) {
          mirror_pos = initalrotate["pos_config_mirror"];
     }     
     if (mirror_pos > -1) {
          if (mirror) {
               config_split[mirror_pos] = "InitialMirror = true";
          }
          else {
               config_split[mirror_pos] = "InitialMirror = false";
          }
     }
     else {
          if (mirror) {       // neue Zeile muss an der richtigen Stelle eingefügt werden - hier direct nach [Alignment]
               var aktline = 0;

               while (aktline < config_split.length){
                    if (config_split[aktline].trim() == "[Alignment]") {
                         break;
                    }
                    aktline++
               }

               // fuege neue Zeile in config_split ein
               var zw = config_split[config_split.length-1];
               config_split.push(zw);
               for (var j = config_split.length-2; j > aktline + 1; --j){
                    config_split[j] = config_split[j-1];
               }

               config_split[aktline + 1] = "InitialMirror = True"
          }
     }

     SaveConfigToServer(_basepath);
}

function UpdateConfigReference(zw, _basepath){
     for (var index = 0; index < 2; ++index)
     {
          var zeile = zw[index]["name"] + " " + zw[index]["x"] + " " + zw[index]["y"];
          var _pos = zw[index]["pos_ref"];
          config_split[_pos] = zeile;

          _filenamenach = ref[index]["name"];
          _filenamevon = _filenamenach.replace("/config/", "/img_tmp/");
          FileDeleteOnServer(_filenamenach, _basepath);
          FileCopyOnServer(_filenamevon, _filenamenach, _basepath);
     
          _filenamenach = _filenamenach.replace(".jpg", "_org.jpg");
          _filenamevon = _filenamevon.replace(".jpg", "_org.jpg");
          FileDeleteOnServer(_filenamenach, _basepath);
          FileCopyOnServer(_filenamevon, _filenamenach, _basepath);

     }

     SaveConfigToServer(_basepath);
}

function MakeContrastImageZW(zw, _enhance, _basepath){
     _filename = zw["name"].replace("/config/", "/img_tmp/");
     url = _basepath + "/editflow.html?task=cutref&in=/config/reference.jpg&out=" + _filename + "&x=" + zw["x"] + "&y="  + zw["y"] + "&dx=" + zw["dx"] + "&dy=" + zw["dy"];
     if (_enhance == true){
          url = url + "&enhance=true";
     }

     var xhttp = new XMLHttpRequest();  
     try {
          xhttp.open("GET", url, false);
          xhttp.send();     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }
}


function GetReferenceSize(name){
     img = new Image();
     var xhttp = new XMLHttpRequest();
			
     url = "http://192.168.178.22/fileserver" + name;
     xhttp.open("GET", url, false);
     xhttp.send();

     var response = xhttp.responseText;
     var binary = ""
     
     for (var responseText = xhttp.responseText, responseTextLen = responseText.length, binary = "", i = 0; i < responseTextLen; ++i) {
          binary += String.fromCharCode(responseText.charCodeAt(i) & 255)
        }
     img.src = 'data:image/jpeg;base64,'+ window.btoa(binary);     

     return [img.width, img.height];
}

	 
function getConfig() {
	return config_gesamt;
     }
     


function dataURLtoBlob(dataurl) {
     var arr = dataurl.split(','), mime = arr[0].match(/:(.*?);/)[1],
          bstr = atob(arr[1]), n = bstr.length, u8arr = new Uint8Array(n);
     while(n--){
          u8arr[n] = bstr.charCodeAt(n);
     }
     return new Blob([u8arr], {type:mime});
     }	

                    
function SaveReferenceImage(_id_canvas, _filename, _doDelete, _basepath = ""){
     if (_doDelete){
          FileDeleteOnServer(_filename, _basepath);
     }

     var canvas = document.getElementById(_id_canvas);
     var JPEG_QUALITY=0.8;
     var dataUrl = canvas.toDataURL('image/jpeg', JPEG_QUALITY);	
     var rtn = dataURLtoBlob(dataUrl);	
     if (!FileSendContent(rtn, _filename, _basepath)){
          alert("Error on saving reference image (" + _filename + ")!\nPlease retry.");
          location.reload();  
     };  
}
