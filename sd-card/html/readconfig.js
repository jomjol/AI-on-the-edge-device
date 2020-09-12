function readconfig_Version(){
     return "1.0.0 - 20200910";
 }

var config_gesamt;
var config_split;
var ref = new Array(2);
var digit = new Array(0);
var analog = new Array(0);
var initalrotate = new Object();

function MakeRefZW(zw, _basepath){
     url = _basepath + "/editflow.html?task=cutref&in=/config/reference.jpg&out=/img_tmp/ref_zw_org.jpg&x=" + zw["x"] + "&y="  + zw["y"] + "&dx=" + zw["dx"] + "&dy=" + zw["dy"];
     var xhttp = new XMLHttpRequest();  
     try {
          xhttp.open("GET", url, false);
          xhttp.send();     }
     catch (error)
     {
//          alert("Deleting Config.ini failed");
     }  
     FileCopyOnServer("/img_tmp/ref_zw_org.jpg", "/img_tmp/ref_zw.jpg", _basepath);
}

function GetCoordinates(index, _basepath){
     FileCopyOnServer(ref[index]["name"], "/img_tmp/ref_zw.jpg", _basepath);

     FileDeleteOnServer("/img_tmp/ref_zw_org.jpg", _basepath);
     var namezw = ref[index]["name"].replace(".jpg", "_org.jpg");
     FileCopyOnServer(namezw, "/img_tmp/ref_zw_org.jpg", _basepath);

     return ref[index];
}

function ParseConfigAlignment(_aktline){
     var akt_ref = 0;
     ++_aktline;

     while ((akt_ref < 2) && (_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
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

     while ((_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
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


function ParseConfigAnalog(_aktline){
     ++_aktline;
     analog.length = 0;

     while ((_aktline < config_split.length) && (config_split[_aktline][0] != "[")) {
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

function SaveROIToConfig(_ROIInfo, _typeROI, _basepath){
     if (_typeROI == "[Digits]"){
          targetROI = digit;
     }
     if (_typeROI == "[Analog]"){
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
          config_split[linewrite] = _ROIInfo[i]["name"] + ", " + _ROIInfo[i]["x"] + ", " + _ROIInfo[i]["y"] + ", " + _ROIInfo[i]["dx"] + ", " + _ROIInfo[i]["dy"];
     }

     SaveConfigToServer(_basepath);
}


function ParseConfig() {
     config_split = config_gesamt.split("\n");
     var aktline = 0;

     while (aktline < config_split.length){
          if (config_split[aktline].trim().toUpperCase() == "[ALIGNMENT]") {
               aktline = ParseConfigAlignment(aktline);
               continue;
          }
          if (config_split[aktline].trim().toUpperCase() == "[DIGITS]") {
               aktline = ParseConfigDigit(aktline);
               continue;
          }

          if (config_split[aktline].trim().toUpperCase() == "[ANALOG]") {
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
          var zeile = ref[_index]["name"] + " " + ref[_index]["x"] + ", " + ref[_index]["y"];
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
               config_split[mirror_pos] = "InitialMirror = True";
          }
          else {
               config_split[mirror_pos] = "InitialMirror = False";
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

function UpdateConfig(zw, _index, _enhance, _basepath){
     var zeile = zw["name"] + " " + zw["x"] + ", " + zw["y"];
     var _pos = ref[_index]["pos_ref"];
     config_split[_pos] = zeile;

     SaveConfigToServer(_basepath);

     var namezw = zw["name"];
     FileCopyOnServer("/img_tmp/ref_zw.jpg", namezw, _basepath);
     var namezw = zw["name"].replace(".jpg", "_org.jpg");
     FileCopyOnServer("/img_tmp/ref_zw_org.jpg", namezw, _basepath);     
}

function MakeContrastImageZW(zw, _enhance, _basepath){
     url = _basepath + "/editflow.html?task=cutref&in=/config/reference.jpg&out=/img_tmp/ref_zw.jpg" + "&x=" + zw["x"] + "&y="  + zw["y"] + "&dx=" + zw["dx"] + "&dy=" + zw["dy"];
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


function ZerlegeZeile(input)
     {
          var Output = Array(0);
          delimiter = " =,";
     
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
     try {
          url = _basepath + '/fileserver/config/config.ini';     
          xhttp.open("GET", url, false);
          xhttp.send();
          config_gesamt = xhttp.responseText;
     }
     catch (error)
     {
     //          alert("Deleting Config.ini failed");
     }
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
