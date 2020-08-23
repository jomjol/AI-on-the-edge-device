var config_gesamt;
var config_split;
var ref = new Array(2);

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

function ParseReference() {
     config_split = config_gesamt.split("\n");
     var i = 0;

     for (var i in config_split) {
          if (config_split[i].trim() == "[Alignment]")
               break;
     }

     if (i >= config_split.length){
          return;
     }

     var akt_ref = 0;
     ++i;

     while ((akt_ref < 2) && (i < config_split.length) && (config_split[i][0] != "[")) {
          var linesplit = ZerlegeZeile(config_split[i]);
          if (linesplit.length == 3)
          {
               ref[akt_ref] = new Object();
               ref[akt_ref]["pos_ref"] = i;
               ref[akt_ref]["name"] = linesplit[0];
               ref[akt_ref]["x"] = linesplit[1];
               ref[akt_ref]["y"] = linesplit[2];
               akt_ref++;
          }
          ++i;
     }
}

function UpdateConfig(zw, _index, _enhance, _basepath){
     var zeile = zw["name"] + " " + zw["x"] + ", " + zw["y"];
     var _pos = ref[_index]["pos_ref"];
     config_split[_pos] = zeile;
     FileDeleteOnServer("/config/config.ini", _basepath);

     var config_gesamt = "";
     for (var i = 0; i < config_split.length; ++i)
     {
          config_gesamt = config_gesamt + config_split[i] + "\n";
     } 

     var rtn = new Blob([config_gesamt], {type: 'mime'});

     FileSendContent(config_gesamt, "/config/config.ini", _basepath);

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
     

     
function loadConfigJS(_basepath) {
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
	return config_txt;
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
                    location.reload()
               } else {
//                    alert(xhttp.status + " Error!\n" + xhttp.responseText);
                    location.reload()
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
