var Module = typeof Module !== "undefined" ? Module : {};

if (!Module.expectedDataFileDownloads) {
 Module.expectedDataFileDownloads = 0;
}

Module.expectedDataFileDownloads++;

(function() {
 var loadPackage = function(metadata) {
  var PACKAGE_PATH = "";
  if (typeof window === "object") {
   PACKAGE_PATH = window["encodeURIComponent"](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf("/")) + "/");
  } else if (typeof process === "undefined" && typeof location !== "undefined") {
   PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf("/")) + "/");
  }
  var PACKAGE_NAME = "webc2e.data";
  var REMOTE_PACKAGE_BASE = "webc2e.data";
  if (typeof Module["locateFilePackage"] === "function" && !Module["locateFile"]) {
   Module["locateFile"] = Module["locateFilePackage"];
   err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)");
  }
  var REMOTE_PACKAGE_NAME = Module["locateFile"] ? Module["locateFile"](REMOTE_PACKAGE_BASE, "") : REMOTE_PACKAGE_BASE;
  var REMOTE_PACKAGE_SIZE = metadata["remote_package_size"];
  var PACKAGE_UUID = metadata["package_uuid"];
  function fetchRemotePackage(packageName, packageSize, callback, errback) {
   if (typeof process === "object" && typeof process.versions === "object" && typeof process.versions.node === "string") {
    require("fs").readFile(packageName, function(err, contents) {
     if (err) {
      errback(err);
     } else {
      callback(contents.buffer);
     }
    });
    return;
   }
   var xhr = new XMLHttpRequest();
   xhr.open("GET", packageName, true);
   xhr.responseType = "arraybuffer";
   xhr.onprogress = function(event) {
    var url = packageName;
    var size = packageSize;
    if (event.total) size = event.total;
    if (event.loaded) {
     if (!xhr.addedTotal) {
      xhr.addedTotal = true;
      if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
      Module.dataFileDownloads[url] = {
       loaded: event.loaded,
       total: size
      };
     } else {
      Module.dataFileDownloads[url].loaded = event.loaded;
     }
     var total = 0;
     var loaded = 0;
     var num = 0;
     for (var download in Module.dataFileDownloads) {
      var data = Module.dataFileDownloads[download];
      total += data.total;
      loaded += data.loaded;
      num++;
     }
     total = Math.ceil(total * Module.expectedDataFileDownloads / num);
     if (Module["setStatus"]) Module["setStatus"]("Downloading data... (" + loaded + "/" + total + ")");
    } else if (!Module.dataFileDownloads) {
     if (Module["setStatus"]) Module["setStatus"]("Downloading data...");
    }
   };
   xhr.onerror = function(event) {
    throw new Error("NetworkError for: " + packageName);
   };
   xhr.onload = function(event) {
    if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || xhr.status == 0 && xhr.response) {
     var packageData = xhr.response;
     callback(packageData);
    } else {
     throw new Error(xhr.statusText + " : " + xhr.responseURL);
    }
   };
   xhr.send(null);
  }
  function handleError(error) {
   console.error("package error:", error);
  }
  var fetchedCallback = null;
  var fetched = Module["getPreloadedPackage"] ? Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;
  if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
   if (fetchedCallback) {
    fetchedCallback(data);
    fetchedCallback = null;
   } else {
    fetched = data;
   }
  }, handleError);
  function runWithFS() {
   function assert(check, msg) {
    if (!check) throw msg + new Error().stack;
   }
   Module["FS_createPath"]("/", "Body Data", true, true);
   Module["FS_createPath"]("/", "Bootstrap", true, true);
   Module["FS_createPath"]("/Bootstrap", "000 Switcher", true, true);
   Module["FS_createPath"]("/Bootstrap", "001 World", true, true);
   Module["FS_createPath"]("/", "Images", true, true);
   Module["FS_createPath"]("/", "My Agents", true, true);
   Module["FS_createPath"]("/", "Sounds", true, true);
   Module["FS_createPath"]("/", "Backgrounds", true, true);
   Module["FS_createPath"]("/", "Overlay Data", true, true);
   Module["FS_createPath"]("/", "Genetics", true, true);
   Module["FS_createPath"]("/", "Catalogue", true, true);
   function DataRequest(start, end, audio) {
    this.start = start;
    this.end = end;
    this.audio = audio;
   }
   DataRequest.prototype = {
    requests: {},
    open: function(mode, name) {
     this.name = name;
     this.requests[name] = this;
     Module["addRunDependency"]("fp " + this.name);
    },
    send: function() {},
    onload: function() {
     var byteArray = this.byteArray.subarray(this.start, this.end);
     this.finish(byteArray);
    },
    finish: function(byteArray) {
     var that = this;
     Module["FS_createDataFile"](this.name, null, byteArray, true, true, true);
     Module["removeRunDependency"]("fp " + that.name);
     this.requests[this.name] = null;
    }
   };
   var files = metadata["files"];
   for (var i = 0; i < files.length; ++i) {
    new DataRequest(files[i]["start"], files[i]["end"], files[i]["audio"] || 0).open("GET", files[i]["filename"]);
   }
   function processPackageData(arrayBuffer) {
    assert(arrayBuffer, "Loading data file failed.");
    assert(arrayBuffer instanceof ArrayBuffer, "bad input to processPackageData");
    var byteArray = new Uint8Array(arrayBuffer);
    var curr;
    DataRequest.prototype.byteArray = byteArray;
    var files = metadata["files"];
    for (var i = 0; i < files.length; ++i) {
     DataRequest.prototype.requests[files[i].filename].onload();
    }
    Module["removeRunDependency"]("datafile_webc2e.data");
   }
   Module["addRunDependency"]("datafile_webc2e.data");
   if (!Module.preloadResults) Module.preloadResults = {};
   Module.preloadResults[PACKAGE_NAME] = {
    fromCache: false
   };
   if (fetched) {
    processPackageData(fetched);
    fetched = null;
   } else {
    fetchedCallback = processPackageData;
   }
  }
  if (Module["calledRun"]) {
   runWithFS();
  } else {
   if (!Module["preRun"]) Module["preRun"] = [];
   Module["preRun"].push(runWithFS);
  }
 };
 loadPackage({
  "files": [ {
   "filename": "/machine.cfg",
   "start": 0,
   "end": 1380
  }, {
   "filename": "/de.xpm",
   "start": 1380,
   "end": 4812
  }, {
   "filename": "/it.xpm",
   "start": 4812,
   "end": 8320
  }, {
   "filename": "/es.xpm",
   "start": 8320,
   "end": 20082
  }, {
   "filename": "/.DS_Store",
   "start": 20082,
   "end": 30326
  }, {
   "filename": "/langpick.conf",
   "start": 30326,
   "end": 30719
  }, {
   "filename": "/langmovie.conf",
   "start": 30719,
   "end": 31073
  }, {
   "filename": "/build_number",
   "start": 31073,
   "end": 31077
  }, {
   "filename": "/Creatures 3.ico",
   "start": 31077,
   "end": 35787
  }, {
   "filename": "/creatures3.bmp",
   "start": 35787,
   "end": 38913
  }, {
   "filename": "/user.cfg",
   "start": 38913,
   "end": 39080
  }, {
   "filename": "/nl.xpm",
   "start": 39080,
   "end": 42558
  }, {
   "filename": "/fr.xpm",
   "start": 42558,
   "end": 45960
  }, {
   "filename": "/en.xpm",
   "start": 45960,
   "end": 56010
  }, {
   "filename": "/en-GB.xpm",
   "start": 56010,
   "end": 77180
  }, {
   "filename": "/Body Data/l14a.att",
   "start": 77180,
   "end": 77404
  }, {
   "filename": "/Body Data/a10a.att",
   "start": 77404,
   "end": 77820
  }, {
   "filename": "/Body Data/m10a.att",
   "start": 77820,
   "end": 78029
  }, {
   "filename": "/Body Data/l02a.att",
   "start": 78029,
   "end": 78239
  }, {
   "filename": "/Body Data/k00a.att",
   "start": 78239,
   "end": 78431
  }, {
   "filename": "/Body Data/f04a.att",
   "start": 78431,
   "end": 78654
  }, {
   "filename": "/Body Data/j12a.att",
   "start": 78654,
   "end": 78878
  }, {
   "filename": "/Body Data/f12a.att",
   "start": 78878,
   "end": 79100
  }, {
   "filename": "/Body Data/j04a.att",
   "start": 79100,
   "end": 79317
  }, {
   "filename": "/Body Data/g00a.att",
   "start": 79317,
   "end": 79524
  }, {
   "filename": "/Body Data/j24a.att",
   "start": 79524,
   "end": 79734
  }, {
   "filename": "/Body Data/g20a.att",
   "start": 79734,
   "end": 79941
  }, {
   "filename": "/Body Data/k20a.att",
   "start": 79941,
   "end": 80133
  }, {
   "filename": "/Body Data/f24a.att",
   "start": 80133,
   "end": 80355
  }, {
   "filename": "/Body Data/l22a.att",
   "start": 80355,
   "end": 80564
  }, {
   "filename": "/Body Data/d24a.att",
   "start": 80564,
   "end": 80778
  }, {
   "filename": "/Body Data/i20a.att",
   "start": 80778,
   "end": 80970
  }, {
   "filename": "/Body Data/e20a.att",
   "start": 80970,
   "end": 81188
  }, {
   "filename": "/Body Data/z22a.att",
   "start": 81188,
   "end": 81316
  }, {
   "filename": "/Body Data/h24a.att",
   "start": 81316,
   "end": 81536
  }, {
   "filename": "/Body Data/b22a.att",
   "start": 81536,
   "end": 82130
  }, {
   "filename": "/Body Data/n22a.att",
   "start": 82130,
   "end": 82334
  }, {
   "filename": "/Body Data/b14a.att",
   "start": 82334,
   "end": 82942
  }, {
   "filename": "/Body Data/n02a.att",
   "start": 82942,
   "end": 83150
  }, {
   "filename": "/Body Data/c10a.att",
   "start": 83150,
   "end": 83357
  }, {
   "filename": "/Body Data/b02a.att",
   "start": 83357,
   "end": 83953
  }, {
   "filename": "/Body Data/n14a.att",
   "start": 83953,
   "end": 84175
  }, {
   "filename": "/Body Data/e00a.att",
   "start": 84175,
   "end": 84393
  }, {
   "filename": "/Body Data/h04a.att",
   "start": 84393,
   "end": 84613
  }, {
   "filename": "/Body Data/z02a.att",
   "start": 84613,
   "end": 84741
  }, {
   "filename": "/Body Data/d12a.att",
   "start": 84741,
   "end": 84955
  }, {
   "filename": "/Body Data/h12a.att",
   "start": 84955,
   "end": 85178
  }, {
   "filename": "/Body Data/d04a.att",
   "start": 85178,
   "end": 85399
  }, {
   "filename": "/Body Data/z14a.att",
   "start": 85399,
   "end": 85527
  }, {
   "filename": "/Body Data/i00a.att",
   "start": 85527,
   "end": 85719
  }, {
   "filename": "/Body Data/j22a.att",
   "start": 85719,
   "end": 85928
  }, {
   "filename": "/Body Data/f22a.att",
   "start": 85928,
   "end": 86141
  }, {
   "filename": "/Body Data/a20a.att",
   "start": 86141,
   "end": 86557
  }, {
   "filename": "/Body Data/l24a.att",
   "start": 86557,
   "end": 86767
  }, {
   "filename": "/Body Data/m20a.att",
   "start": 86767,
   "end": 86950
  }, {
   "filename": "/Body Data/l12a.att",
   "start": 86950,
   "end": 87174
  }, {
   "filename": "/Body Data/m00a.att",
   "start": 87174,
   "end": 87357
  }, {
   "filename": "/Body Data/a00a.att",
   "start": 87357,
   "end": 87773
  }, {
   "filename": "/Body Data/l04a.att",
   "start": 87773,
   "end": 87992
  }, {
   "filename": "/Body Data/g10a.att",
   "start": 87992,
   "end": 88199
  }, {
   "filename": "/Body Data/f02a.att",
   "start": 88199,
   "end": 88421
  }, {
   "filename": "/Body Data/j14a.att",
   "start": 88421,
   "end": 88645
  }, {
   "filename": "/Body Data/f14a.att",
   "start": 88645,
   "end": 88868
  }, {
   "filename": "/Body Data/j02a.att",
   "start": 88868,
   "end": 89078
  }, {
   "filename": "/Body Data/k10a.att",
   "start": 89078,
   "end": 89271
  }, {
   "filename": "/Body Data/b12a.att",
   "start": 89271,
   "end": 89879
  }, {
   "filename": "/Body Data/n04a.att",
   "start": 89879,
   "end": 90088
  }, {
   "filename": "/Body Data/c00a.att",
   "start": 90088,
   "end": 90295
  }, {
   "filename": "/Body Data/b04a.att",
   "start": 90295,
   "end": 90896
  }, {
   "filename": "/Body Data/n12a.att",
   "start": 90896,
   "end": 91118
  }, {
   "filename": "/Body Data/i10a.att",
   "start": 91118,
   "end": 91311
  }, {
   "filename": "/Body Data/h02a.att",
   "start": 91311,
   "end": 91531
  }, {
   "filename": "/Body Data/z04a.att",
   "start": 91531,
   "end": 91659
  }, {
   "filename": "/Body Data/d14a.att",
   "start": 91659,
   "end": 91880
  }, {
   "filename": "/Body Data/h14a.att",
   "start": 91880,
   "end": 92104
  }, {
   "filename": "/Body Data/d02a.att",
   "start": 92104,
   "end": 92318
  }, {
   "filename": "/Body Data/z12a.att",
   "start": 92318,
   "end": 92446
  }, {
   "filename": "/Body Data/e10a.att",
   "start": 92446,
   "end": 92666
  }, {
   "filename": "/Body Data/d22a.att",
   "start": 92666,
   "end": 92879
  }, {
   "filename": "/Body Data/z24a.att",
   "start": 92879,
   "end": 93007
  }, {
   "filename": "/Body Data/h22a.att",
   "start": 93007,
   "end": 93227
  }, {
   "filename": "/Body Data/b24a.att",
   "start": 93227,
   "end": 93823
  }, {
   "filename": "/Body Data/n24a.att",
   "start": 93823,
   "end": 94031
  }, {
   "filename": "/Body Data/c20a.att",
   "start": 94031,
   "end": 94238
  }, {
   "filename": "/Body Data/m22a.att",
   "start": 94238,
   "end": 94442
  }, {
   "filename": "/Body Data/a22a.att",
   "start": 94442,
   "end": 94858
  }, {
   "filename": "/Body Data/f20a.att",
   "start": 94858,
   "end": 95065
  }, {
   "filename": "/Body Data/k24a.att",
   "start": 95065,
   "end": 95266
  }, {
   "filename": "/Body Data/g24a.att",
   "start": 95266,
   "end": 95480
  }, {
   "filename": "/Body Data/j20a.att",
   "start": 95480,
   "end": 95686
  }, {
   "filename": "/Body Data/g04a.att",
   "start": 95686,
   "end": 95908
  }, {
   "filename": "/Body Data/k12a.att",
   "start": 95908,
   "end": 96119
  }, {
   "filename": "/Body Data/j00a.att",
   "start": 96119,
   "end": 96325
  }, {
   "filename": "/Body Data/f00a.att",
   "start": 96325,
   "end": 96532
  }, {
   "filename": "/Body Data/g12a.att",
   "start": 96532,
   "end": 96746
  }, {
   "filename": "/Body Data/k04a.att",
   "start": 96746,
   "end": 96969
  }, {
   "filename": "/Body Data/m14a.att",
   "start": 96969,
   "end": 97187
  }, {
   "filename": "/Body Data/a02a.att",
   "start": 97187,
   "end": 97603
  }, {
   "filename": "/Body Data/m02a.att",
   "start": 97603,
   "end": 97812
  }, {
   "filename": "/Body Data/a14a.att",
   "start": 97812,
   "end": 98228
  }, {
   "filename": "/Body Data/l10a.att",
   "start": 98228,
   "end": 98438
  }, {
   "filename": "/Body Data/i04a.att",
   "start": 98438,
   "end": 98661
  }, {
   "filename": "/Body Data/e12a.att",
   "start": 98661,
   "end": 98884
  }, {
   "filename": "/Body Data/d00a.att",
   "start": 98884,
   "end": 99091
  }, {
   "filename": "/Body Data/z10a.att",
   "start": 99091,
   "end": 99219
  }, {
   "filename": "/Body Data/h00a.att",
   "start": 99219,
   "end": 99437
  }, {
   "filename": "/Body Data/i12a.att",
   "start": 99437,
   "end": 99651
  }, {
   "filename": "/Body Data/e04a.att",
   "start": 99651,
   "end": 99871
  }, {
   "filename": "/Body Data/n10a.att",
   "start": 99871,
   "end": 100081
  }, {
   "filename": "/Body Data/c14a.att",
   "start": 100081,
   "end": 100304
  }, {
   "filename": "/Body Data/c02a.att",
   "start": 100304,
   "end": 100526
  }, {
   "filename": "/Body Data/b10a.att",
   "start": 100526,
   "end": 101134
  }, {
   "filename": "/Body Data/c22a.att",
   "start": 101134,
   "end": 101347
  }, {
   "filename": "/Body Data/h20a.att",
   "start": 101347,
   "end": 101565
  }, {
   "filename": "/Body Data/e24a.att",
   "start": 101565,
   "end": 101785
  }, {
   "filename": "/Body Data/i24a.att",
   "start": 101785,
   "end": 101986
  }, {
   "filename": "/Body Data/d20a.att",
   "start": 101986,
   "end": 102193
  }, {
   "filename": "/Body Data/g02a.att",
   "start": 102193,
   "end": 102407
  }, {
   "filename": "/Body Data/k14a.att",
   "start": 102407,
   "end": 102631
  }, {
   "filename": "/Body Data/f10a.att",
   "start": 102631,
   "end": 102838
  }, {
   "filename": "/Body Data/j10a.att",
   "start": 102838,
   "end": 103048
  }, {
   "filename": "/Body Data/g14a.att",
   "start": 103048,
   "end": 103270
  }, {
   "filename": "/Body Data/k02a.att",
   "start": 103270,
   "end": 103471
  }, {
   "filename": "/Body Data/l00a.att",
   "start": 103471,
   "end": 103677
  }, {
   "filename": "/Body Data/m12a.att",
   "start": 103677,
   "end": 103887
  }, {
   "filename": "/Body Data/a04a.att",
   "start": 103887,
   "end": 104303
  }, {
   "filename": "/Body Data/m04a.att",
   "start": 104303,
   "end": 104517
  }, {
   "filename": "/Body Data/a12a.att",
   "start": 104517,
   "end": 104933
  }, {
   "filename": "/Body Data/m24a.att",
   "start": 104933,
   "end": 105142
  }, {
   "filename": "/Body Data/l20a.att",
   "start": 105142,
   "end": 105348
  }, {
   "filename": "/Body Data/a24a.att",
   "start": 105348,
   "end": 105764
  }, {
   "filename": "/Body Data/k22a.att",
   "start": 105764,
   "end": 105958
  }, {
   "filename": "/Body Data/g22a.att",
   "start": 105958,
   "end": 106171
  }, {
   "filename": "/Body Data/c24a.att",
   "start": 106171,
   "end": 106393
  }, {
   "filename": "/Body Data/n20a.att",
   "start": 106393,
   "end": 106581
  }, {
   "filename": "/Body Data/b20a.att",
   "start": 106581,
   "end": 107169
  }, {
   "filename": "/Body Data/z20a.att",
   "start": 107169,
   "end": 107297
  }, {
   "filename": "/Body Data/e22a.att",
   "start": 107297,
   "end": 107517
  }, {
   "filename": "/Body Data/i22a.att",
   "start": 107517,
   "end": 107711
  }, {
   "filename": "/Body Data/i02a.att",
   "start": 107711,
   "end": 107912
  }, {
   "filename": "/Body Data/e14a.att",
   "start": 107912,
   "end": 108136
  }, {
   "filename": "/Body Data/h10a.att",
   "start": 108136,
   "end": 108356
  }, {
   "filename": "/Body Data/z00a.att",
   "start": 108356,
   "end": 108484
  }, {
   "filename": "/Body Data/d10a.att",
   "start": 108484,
   "end": 108691
  }, {
   "filename": "/Body Data/i14a.att",
   "start": 108691,
   "end": 108915
  }, {
   "filename": "/Body Data/e02a.att",
   "start": 108915,
   "end": 109135
  }, {
   "filename": "/Body Data/b00a.att",
   "start": 109135,
   "end": 109723
  }, {
   "filename": "/Body Data/c12a.att",
   "start": 109723,
   "end": 109945
  }, {
   "filename": "/Body Data/c04a.att",
   "start": 109945,
   "end": 110168
  }, {
   "filename": "/Body Data/n00a.att",
   "start": 110168,
   "end": 110356
  }, {
   "filename": "/Bootstrap/.DS_Store",
   "start": 110356,
   "end": 116504
  }, {
   "filename": "/Bootstrap/000 Switcher/disable debug keys.cos",
   "start": 116504,
   "end": 116537
  }, {
   "filename": "/Bootstrap/000 Switcher/world switcher.cos",
   "start": 116537,
   "end": 128022
  }, {
   "filename": "/Bootstrap/000 Switcher/!map.cos",
   "start": 128022,
   "end": 128110
  }, {
   "filename": "/Bootstrap/001 World/creature history.cos",
   "start": 128110,
   "end": 147405
  }, {
   "filename": "/Bootstrap/001 World/apples.cos",
   "start": 147405,
   "end": 149578
  }, {
   "filename": "/Bootstrap/001 World/plant model - foxglove plant.cos",
   "start": 149578,
   "end": 154235
  }, {
   "filename": "/Bootstrap/001 World/fungi.cos",
   "start": 154235,
   "end": 158038
  }, {
   "filename": "/Bootstrap/001 World/logicgate.cos",
   "start": 158038,
   "end": 160146
  }, {
   "filename": "/Bootstrap/001 World/garbage spewer.cos",
   "start": 160146,
   "end": 160884
  }, {
   "filename": "/Bootstrap/001 World/creaturedoneto.cos",
   "start": 160884,
   "end": 163424
  }, {
   "filename": "/Bootstrap/001 World/disable debug keys.cos",
   "start": 163424,
   "end": 163457
  }, {
   "filename": "/Bootstrap/001 World/handlefish.cos",
   "start": 163457,
   "end": 183131
  }, {
   "filename": "/Bootstrap/001 World/radio machines.cos",
   "start": 183131,
   "end": 184815
  }, {
   "filename": "/Bootstrap/001 World/medicine maker.cos",
   "start": 184815,
   "end": 196619
  }, {
   "filename": "/Bootstrap/001 World/aquatic tunnel gates.cos",
   "start": 196619,
   "end": 200361
  }, {
   "filename": "/Bootstrap/001 World/gui 1.cos",
   "start": 200361,
   "end": 227794
  }, {
   "filename": "/Bootstrap/001 World/smell emitter + detector.cos",
   "start": 227794,
   "end": 231862
  }, {
   "filename": "/Bootstrap/001 World/anti-bacterial spray.cos",
   "start": 231862,
   "end": 233632
  }, {
   "filename": "/Bootstrap/001 World/grendel and ettin egg maker.cos",
   "start": 233632,
   "end": 235086
  }, {
   "filename": "/Bootstrap/001 World/recycler2.cos",
   "start": 235086,
   "end": 238170
  }, {
   "filename": "/Bootstrap/001 World/pregnancy indication device.cos",
   "start": 238170,
   "end": 240664
  }, {
   "filename": "/Bootstrap/001 World/pointer scripts.cos",
   "start": 240664,
   "end": 242890
  }, {
   "filename": "/Bootstrap/001 World/output display device.cos",
   "start": 242890,
   "end": 243445
  }, {
   "filename": "/Bootstrap/001 World/pumperspikel.cos",
   "start": 243445,
   "end": 245702
  }, {
   "filename": "/Bootstrap/001 World/medical pod and screens.cos",
   "start": 245702,
   "end": 279754
  }, {
   "filename": "/Bootstrap/001 World/ball.cos",
   "start": 279754,
   "end": 281219
  }, {
   "filename": "/Bootstrap/001 World/norn egg layer.cos",
   "start": 281219,
   "end": 286723
  }, {
   "filename": "/Bootstrap/001 World/xbioenergy incrementer.cos",
   "start": 286723,
   "end": 286978
  }, {
   "filename": "/Bootstrap/001 World/opal sponge.cos",
   "start": 286978,
   "end": 292143
  }, {
   "filename": "/Bootstrap/001 World/wasp.cos",
   "start": 292143,
   "end": 297787
  }, {
   "filename": "/Bootstrap/001 World/engineering scenery.cos",
   "start": 297787,
   "end": 297891
  }, {
   "filename": "/Bootstrap/001 World/magic profiler.cos",
   "start": 297891,
   "end": 299253
  }, {
   "filename": "/Bootstrap/001 World/knowledge stones.cos",
   "start": 299253,
   "end": 300257
  }, {
   "filename": "/Bootstrap/001 World/angel fish.cos",
   "start": 300257,
   "end": 316574
  }, {
   "filename": "/Bootstrap/001 World/ettin accessways.cos",
   "start": 316574,
   "end": 321836
  }, {
   "filename": "/Bootstrap/001 World/aquatic_population_monitoring_device.cos",
   "start": 321836,
   "end": 325744
  }, {
   "filename": "/Bootstrap/001 World/internal terrarium door.cos",
   "start": 325744,
   "end": 328028
  }, {
   "filename": "/Bootstrap/001 World/numeric output tool.cos",
   "start": 328028,
   "end": 330671
  }, {
   "filename": "/Bootstrap/001 World/gene pod.cos",
   "start": 330671,
   "end": 333750
  }, {
   "filename": "/Bootstrap/001 World/norn cutaways 01.cos",
   "start": 333750,
   "end": 334052
  }, {
   "filename": "/Bootstrap/001 World/dragonfly.cos",
   "start": 334052,
   "end": 352321
  }, {
   "filename": "/Bootstrap/001 World/creaturesaffectinghand.cos",
   "start": 352321,
   "end": 352609
  }, {
   "filename": "/Bootstrap/001 World/beatbox.cos",
   "start": 352609,
   "end": 353928
  }, {
   "filename": "/Bootstrap/001 World/favourite places.cos",
   "start": 353928,
   "end": 363710
  }, {
   "filename": "/Bootstrap/001 World/stars and pickup panel.cos",
   "start": 363710,
   "end": 366111
  }, {
   "filename": "/Bootstrap/001 World/holdinghandscontrolscripts.cos",
   "start": 366111,
   "end": 366379
  }, {
   "filename": "/Bootstrap/001 World/vents.cos",
   "start": 366379,
   "end": 366703
  }, {
   "filename": "/Bootstrap/001 World/waterplant.cos",
   "start": 366703,
   "end": 367790
  }, {
   "filename": "/Bootstrap/001 World/ocean surface.cos",
   "start": 367790,
   "end": 368493
  }, {
   "filename": "/Bootstrap/001 World/rocklice.cos",
   "start": 368493,
   "end": 379425
  }, {
   "filename": "/Bootstrap/001 World/nudibranch.cos",
   "start": 379425,
   "end": 383886
  }, {
   "filename": "/Bootstrap/001 World/norn seed launcher.cos",
   "start": 383886,
   "end": 403105
  }, {
   "filename": "/Bootstrap/001 World/plant model - foxglove leaf.cos",
   "start": 403105,
   "end": 404158
  }, {
   "filename": "/Bootstrap/001 World/toilet control.cos",
   "start": 404158,
   "end": 404440
  }, {
   "filename": "/Bootstrap/001 World/ettin seed bank.cos",
   "start": 404440,
   "end": 410282
  }, {
   "filename": "/Bootstrap/001 World/lifts.cos",
   "start": 410282,
   "end": 428024
  }, {
   "filename": "/Bootstrap/001 World/robin2.cos",
   "start": 428024,
   "end": 450371
  }, {
   "filename": "/Bootstrap/001 World/carrot.cos",
   "start": 450371,
   "end": 453074
  }, {
   "filename": "/Bootstrap/001 World/wolf control.cos",
   "start": 453074,
   "end": 455376
  }, {
   "filename": "/Bootstrap/001 World/wysts.cos",
   "start": 455376,
   "end": 461071
  }, {
   "filename": "/Bootstrap/001 World/water smell emitter.cos",
   "start": 461071,
   "end": 461932
  }, {
   "filename": "/Bootstrap/001 World/all engineering airlock.cos",
   "start": 461932,
   "end": 469428
  }, {
   "filename": "/Bootstrap/001 World/ocean_pod.cos",
   "start": 469428,
   "end": 469522
  }, {
   "filename": "/Bootstrap/001 World/norn atmosphere machine.cos",
   "start": 469522,
   "end": 472337
  }, {
   "filename": "/Bootstrap/001 World/sealift.cos",
   "start": 472337,
   "end": 473183
  }, {
   "filename": "/Bootstrap/001 World/mosquito.cos",
   "start": 473183,
   "end": 476843
  }, {
   "filename": "/Bootstrap/001 World/space toy.cos",
   "start": 476843,
   "end": 477919
  }, {
   "filename": "/Bootstrap/001 World/creaturetocreature.cos",
   "start": 477919,
   "end": 479115
  }, {
   "filename": "/Bootstrap/001 World/neon fish.cos",
   "start": 479115,
   "end": 494342
  }, {
   "filename": "/Bootstrap/001 World/starplant.cos",
   "start": 494342,
   "end": 494685
  }, {
   "filename": "/Bootstrap/001 World/replicator.cos",
   "start": 494685,
   "end": 501050
  }, {
   "filename": "/Bootstrap/001 World/welcome screen.cos",
   "start": 501050,
   "end": 503931
  }, {
   "filename": "/Bootstrap/001 World/grendel seed bank.cos",
   "start": 503931,
   "end": 508806
  }, {
   "filename": "/Bootstrap/001 World/xy tool.cos",
   "start": 508806,
   "end": 510314
  }, {
   "filename": "/Bootstrap/001 World/textentry.cos",
   "start": 510314,
   "end": 512675
  }, {
   "filename": "/Bootstrap/001 World/orange sponge.cos",
   "start": 512675,
   "end": 517829
  }, {
   "filename": "/Bootstrap/001 World/object of attention indicator.cos",
   "start": 517829,
   "end": 518382
  }, {
   "filename": "/Bootstrap/001 World/ettin area environment.cos",
   "start": 518382,
   "end": 521430
  }, {
   "filename": "/Bootstrap/001 World/z_agent smells.cos",
   "start": 521430,
   "end": 521642
  }, {
   "filename": "/Bootstrap/001 World/grazer2.cos",
   "start": 521642,
   "end": 535676
  }, {
   "filename": "/Bootstrap/001 World/man-o-war.cos",
   "start": 535676,
   "end": 544409
  }, {
   "filename": "/Bootstrap/001 World/count gate.cos",
   "start": 544409,
   "end": 546104
  }, {
   "filename": "/Bootstrap/001 World/butterfly.cos",
   "start": 546104,
   "end": 561841
  }, {
   "filename": "/Bootstrap/001 World/z_creaturesaffectinganimals.cos",
   "start": 561841,
   "end": 562108
  }, {
   "filename": "/Bootstrap/001 World/infinite_cheese_machine.cos",
   "start": 562108,
   "end": 567247
  }, {
   "filename": "/Bootstrap/001 World/antigen detector.cos",
   "start": 567247,
   "end": 569095
  }, {
   "filename": "/Bootstrap/001 World/toolbar.cos",
   "start": 569095,
   "end": 574573
  }, {
   "filename": "/Bootstrap/001 World/corridor doors.cos",
   "start": 574573,
   "end": 603160
  }, {
   "filename": "/Bootstrap/001 World/gumin grass.cos",
   "start": 603160,
   "end": 608258
  }, {
   "filename": "/Bootstrap/001 World/single chemical graphing gadget.cos",
   "start": 608258,
   "end": 612734
  }, {
   "filename": "/Bootstrap/001 World/ant.cos",
   "start": 612734,
   "end": 622931
  }, {
   "filename": "/Bootstrap/001 World/volcano.cos",
   "start": 622931,
   "end": 629251
  }, {
   "filename": "/Bootstrap/001 World/cacbana.cos",
   "start": 629251,
   "end": 637161
  }, {
   "filename": "/Bootstrap/001 World/piranha.cos",
   "start": 637161,
   "end": 643500
  }, {
   "filename": "/Bootstrap/001 World/pointer.cos",
   "start": 643500,
   "end": 643623
  }, {
   "filename": "/Bootstrap/001 World/grendel scenery.cos",
   "start": 643623,
   "end": 643843
  }, {
   "filename": "/Bootstrap/001 World/genetic splicer panel2.cos",
   "start": 643843,
   "end": 648052
  }, {
   "filename": "/Bootstrap/001 World/rock cutaway norn terrarium.cos",
   "start": 648052,
   "end": 648146
  }, {
   "filename": "/Bootstrap/001 World/plant model - foxglove seed.cos",
   "start": 648146,
   "end": 650650
  }, {
   "filename": "/Bootstrap/001 World/agent help.cos",
   "start": 650650,
   "end": 655595
  }, {
   "filename": "/Bootstrap/001 World/bongo_drums.cos",
   "start": 655595,
   "end": 656687
  }, {
   "filename": "/Bootstrap/001 World/sepulchre.cos",
   "start": 656687,
   "end": 657723
  }, {
   "filename": "/Bootstrap/001 World/hummingbird.cos",
   "start": 657723,
   "end": 677512
  }, {
   "filename": "/Bootstrap/001 World/trapdoor.cos",
   "start": 677512,
   "end": 679433
  }, {
   "filename": "/Bootstrap/001 World/bee.cos",
   "start": 679433,
   "end": 690764
  }, {
   "filename": "/Bootstrap/001 World/wetness monitor and designator.cos",
   "start": 690764,
   "end": 691961
  }, {
   "filename": "/Bootstrap/001 World/speech bubble factory.cos",
   "start": 691961,
   "end": 693447
  }, {
   "filename": "/Bootstrap/001 World/hawk.cos",
   "start": 693447,
   "end": 702750
  }, {
   "filename": "/Bootstrap/001 World/gnats.cos",
   "start": 702750,
   "end": 707450
  }, {
   "filename": "/Bootstrap/001 World/aquatic_launcher.cos",
   "start": 707450,
   "end": 716877
  }, {
   "filename": "/Bootstrap/001 World/meerk.cos",
   "start": 716877,
   "end": 721001
  }, {
   "filename": "/Bootstrap/001 World/grass.cos",
   "start": 721001,
   "end": 727297
  }, {
   "filename": "/Bootstrap/001 World/!c3_game variables.cos",
   "start": 727297,
   "end": 728005
  }, {
   "filename": "/Bootstrap/001 World/rainbow_sharkling.cos",
   "start": 728005,
   "end": 736415
  }, {
   "filename": "/Bootstrap/001 World/delay gate.cos",
   "start": 736415,
   "end": 739365
  }, {
   "filename": "/Bootstrap/001 World/marine_airlocks.cos",
   "start": 739365,
   "end": 744985
  }, {
   "filename": "/Bootstrap/001 World/aquamites.cos",
   "start": 744985,
   "end": 747459
  }, {
   "filename": "/Bootstrap/001 World/blueprint agent scripts.cos",
   "start": 747459,
   "end": 751511
  }, {
   "filename": "/Bootstrap/001 World/creaturebubbles.cos",
   "start": 751511,
   "end": 752129
  }, {
   "filename": "/Bootstrap/001 World/desert grass.cos",
   "start": 752129,
   "end": 758712
  }, {
   "filename": "/Bootstrap/001 World/norn indicator.cos",
   "start": 758712,
   "end": 759100
  }, {
   "filename": "/Bootstrap/001 World/grasshopper.cos",
   "start": 759100,
   "end": 765816
  }, {
   "filename": "/Bootstrap/001 World/choppertoy.cos",
   "start": 765816,
   "end": 767767
  }, {
   "filename": "/Bootstrap/001 World/recycler overlay.cos",
   "start": 767767,
   "end": 767872
  }, {
   "filename": "/Bootstrap/001 World/kingfisher.cos",
   "start": 767872,
   "end": 778623
  }, {
   "filename": "/Bootstrap/001 World/marine cave water.cos",
   "start": 778623,
   "end": 781809
  }, {
   "filename": "/Bootstrap/001 World/hoppity.cos",
   "start": 781809,
   "end": 789903
  }, {
   "filename": "/Bootstrap/001 World/not gate.cos",
   "start": 789903,
   "end": 790688
  }, {
   "filename": "/Bootstrap/001 World/stickleback.cos",
   "start": 790688,
   "end": 799889
  }, {
   "filename": "/Bootstrap/001 World/scroll settings.cos",
   "start": 799889,
   "end": 799947
  }, {
   "filename": "/Bootstrap/001 World/pick-ups.cos",
   "start": 799947,
   "end": 802877
  }, {
   "filename": "/Bootstrap/001 World/switch.cos",
   "start": 802877,
   "end": 804076
  }, {
   "filename": "/Bootstrap/001 World/lung.cos",
   "start": 804076,
   "end": 804304
  }, {
   "filename": "/Bootstrap/001 World/connecting agent pointer change.cos",
   "start": 804304,
   "end": 804652
  }, {
   "filename": "/Bootstrap/001 World/creatureobjectvariables.cos",
   "start": 804652,
   "end": 804658
  }, {
   "filename": "/Bootstrap/001 World/fixed position fish egg launcher.cos",
   "start": 804658,
   "end": 810788
  }, {
   "filename": "/Bootstrap/001 World/teleporter.cos",
   "start": 810788,
   "end": 816284
  }, {
   "filename": "/Bootstrap/001 World/learning_machine.cos",
   "start": 816284,
   "end": 824518
  }, {
   "filename": "/Bootstrap/001 World/creature detector.cos",
   "start": 824518,
   "end": 828896
  }, {
   "filename": "/Bootstrap/001 World/creator.cos",
   "start": 828896,
   "end": 837134
  }, {
   "filename": "/Bootstrap/001 World/ettin_muscle_paralyser.cos",
   "start": 837134,
   "end": 837906
  }, {
   "filename": "/Bootstrap/001 World/palmplant.cos",
   "start": 837906,
   "end": 838485
  }, {
   "filename": "/Bootstrap/001 World/creatureinvoluntary.cos",
   "start": 838485,
   "end": 843310
  }, {
   "filename": "/Bootstrap/001 World/light & heat emitters nt.cos",
   "start": 843310,
   "end": 848017
  }, {
   "filename": "/Bootstrap/001 World/creaturebreeding.cos",
   "start": 848017,
   "end": 855238
  }, {
   "filename": "/Bootstrap/001 World/aquamite_maker.cos",
   "start": 855238,
   "end": 857783
  }, {
   "filename": "/Bootstrap/001 World/plant model - foxglove flower.cos",
   "start": 857783,
   "end": 858864
  }, {
   "filename": "/Bootstrap/001 World/norn to grendel door.cos",
   "start": 858864,
   "end": 861607
  }, {
   "filename": "/Bootstrap/001 World/crypt3.cos",
   "start": 861607,
   "end": 874032
  }, {
   "filename": "/Bootstrap/001 World/airlock agent.cos",
   "start": 874032,
   "end": 875514
  }, {
   "filename": "/Bootstrap/001 World/woodpigeon2.cos",
   "start": 875514,
   "end": 894156
  }, {
   "filename": "/Bootstrap/001 World/xefficiency indicator.cos",
   "start": 894156,
   "end": 896993
  }, {
   "filename": "/Bootstrap/001 World/keyboard handler.cos",
   "start": 896993,
   "end": 905388
  }, {
   "filename": "/Bootstrap/001 World/!map.cos",
   "start": 905388,
   "end": 990540
  }, {
   "filename": "/Bootstrap/001 World/lightmodule.cos",
   "start": 990540,
   "end": 993838
  }, {
   "filename": "/Bootstrap/001 World/zzz_move camera.cos",
   "start": 993838,
   "end": 993887
  }, {
   "filename": "/Bootstrap/001 World/beetle.cos",
   "start": 993887,
   "end": 999530
  }, {
   "filename": "/Bootstrap/001 World/environmental controls.cos",
   "start": 999530,
   "end": 1006332
  }, {
   "filename": "/Bootstrap/001 World/hover vehicle.cos",
   "start": 1006332,
   "end": 1009572
  }, {
   "filename": "/Bootstrap/001 World/all bridge airlock.cos",
   "start": 1009572,
   "end": 1017481
  }, {
   "filename": "/Bootstrap/001 World/blueprint agent export as pray file.cos",
   "start": 1017481,
   "end": 1019066
  }, {
   "filename": "/Bootstrap/001 World/pointer activate stimming norns.cos",
   "start": 1019066,
   "end": 1019283
  }, {
   "filename": "/Bootstrap/001 World/life event factory.cos",
   "start": 1019283,
   "end": 1022167
  }, {
   "filename": "/Bootstrap/001 World/plusminus switch.cos",
   "start": 1022167,
   "end": 1023084
  }, {
   "filename": "/Bootstrap/001 World/frame rater.cos",
   "start": 1023084,
   "end": 1023916
  }, {
   "filename": "/Bootstrap/001 World/uglee.cos",
   "start": 1023916,
   "end": 1028749
  }, {
   "filename": "/Bootstrap/001 World/camera unit.cos",
   "start": 1028749,
   "end": 1033791
  }, {
   "filename": "/Bootstrap/001 World/lightbox.cos",
   "start": 1033791,
   "end": 1035844
  }, {
   "filename": "/Bootstrap/001 World/bacteria.cos",
   "start": 1035844,
   "end": 1044367
  }, {
   "filename": "/Bootstrap/001 World/sludge gun.cos",
   "start": 1044367,
   "end": 1047149
  }, {
   "filename": "/Bootstrap/001 World/gene splicer overlay.cos",
   "start": 1047149,
   "end": 1047262
  }, {
   "filename": "/Bootstrap/001 World/marine room cutout.cos",
   "start": 1047262,
   "end": 1047381
  }, {
   "filename": "/Bootstrap/001 World/gnarler.cos",
   "start": 1047381,
   "end": 1054127
  }, {
   "filename": "/Bootstrap/001 World/autosave.cos",
   "start": 1054127,
   "end": 1054455
  }, {
   "filename": "/Bootstrap/001 World/death.cos",
   "start": 1054455,
   "end": 1055217
  }, {
   "filename": "/Bootstrap/001 World/hatchery2.cos",
   "start": 1055217,
   "end": 1058919
  }, {
   "filename": "/Bootstrap/001 World/balloon bug.cos",
   "start": 1058919,
   "end": 1063399
  }, {
   "filename": "/Bootstrap/001 World/tendril.cos",
   "start": 1063399,
   "end": 1067051
  }, {
   "filename": "/Bootstrap/001 World/home smell emitters.cos",
   "start": 1067051,
   "end": 1067674
  }, {
   "filename": "/Bootstrap/001 World/contact sensor.cos",
   "start": 1067674,
   "end": 1068511
  }, {
   "filename": "/Bootstrap/001 World/caos command line.cos",
   "start": 1068511,
   "end": 1070995
  }, {
   "filename": "/Bootstrap/001 World/norn platform.cos",
   "start": 1070995,
   "end": 1071136
  }, {
   "filename": "/Bootstrap/001 World/siren.cos",
   "start": 1071136,
   "end": 1072436
  }, {
   "filename": "/Bootstrap/001 World/cuttlefish.cos",
   "start": 1072436,
   "end": 1076513
  }, {
   "filename": "/Bootstrap/001 World/medical scanner.cos",
   "start": 1076513,
   "end": 1079036
  }, {
   "filename": "/Bootstrap/001 World/lung overlay.cos",
   "start": 1079036,
   "end": 1079213
  }, {
   "filename": "/Bootstrap/001 World/clown fish.cos",
   "start": 1079213,
   "end": 1096551
  }, {
   "filename": "/Bootstrap/001 World/grendel area environment.cos",
   "start": 1096551,
   "end": 1101199
  }, {
   "filename": "/Bootstrap/001 World/creaturedecisions.cos",
   "start": 1101199,
   "end": 1108401
  }, {
   "filename": "/Bootstrap/001 World/hedgehog.cos",
   "start": 1108401,
   "end": 1116597
  }, {
   "filename": "/Bootstrap/001 World/kobold.cos",
   "start": 1116597,
   "end": 1123643
  }, {
   "filename": "/Bootstrap/001 World/mediporter.cos",
   "start": 1123643,
   "end": 1125159
  }, {
   "filename": "/Bootstrap/001 World/map display.cos",
   "start": 1125159,
   "end": 1139763
  }, {
   "filename": "/Bootstrap/001 World/snail.cos",
   "start": 1139763,
   "end": 1144134
  }, {
   "filename": "/Bootstrap/001 World/bluebell.cos",
   "start": 1144134,
   "end": 1144746
  }, {
   "filename": "/Images/g00a.c16",
   "start": 1144746,
   "end": 1152980
  }, {
   "filename": "/Images/c50a.c16",
   "start": 1152980,
   "end": 1161700
  }, {
   "filename": "/Images/f45a.c16",
   "start": 1161700,
   "end": 1180126
  }, {
   "filename": "/Images/b15a.c16",
   "start": 1180126,
   "end": 1594076
  }, {
   "filename": "/Images/mediscreen.c16",
   "start": 1594076,
   "end": 2882222
  }, {
   "filename": "/Images/j04a.c16",
   "start": 2882222,
   "end": 2897664
  }, {
   "filename": "/Images/n54a.c16",
   "start": 2897664,
   "end": 2915208
  }, {
   "filename": "/Images/b42a.c16",
   "start": 2915208,
   "end": 3049454
  }, {
   "filename": "/Images/f12a.c16",
   "start": 3049454,
   "end": 3062604
  }, {
   "filename": "/Images/caterpillar.c16",
   "start": 3062604,
   "end": 3129064
  }, {
   "filename": "/Images/n42a.c16",
   "start": 3129064,
   "end": 3138954
  }, {
   "filename": "/Images/j12a.c16",
   "start": 3138954,
   "end": 3159880
  }, {
   "filename": "/Images/062a.c16",
   "start": 3159880,
   "end": 3214222
  }, {
   "filename": "/Images/f04a.c16",
   "start": 3214222,
   "end": 3232648
  }, {
   "filename": "/Images/b54a.c16",
   "start": 3232648,
   "end": 3620738
  }, {
   "filename": "/Images/j45a.c16",
   "start": 3620738,
   "end": 3636180
  }, {
   "filename": "/Images/hoppity.c16",
   "start": 3636180,
   "end": 4265662
  }, {
   "filename": "/Images/n15a.c16",
   "start": 4265662,
   "end": 4283206
  }, {
   "filename": "/Images/k00a.c16",
   "start": 4283206,
   "end": 4288880
  }, {
   "filename": "/Images/042c.c16",
   "start": 4288880,
   "end": 4350902
  }, {
   "filename": "/Images/d44a.c16",
   "start": 4350902,
   "end": 4368194
  }, {
   "filename": "/Images/042b.c16",
   "start": 4368194,
   "end": 4430216
  }, {
   "filename": "/Images/l02a.c16",
   "start": 4430216,
   "end": 4441430
  }, {
   "filename": "/Images/h52a.c16",
   "start": 4441430,
   "end": 4456520
  }, {
   "filename": "/Images/dust cloud.c16",
   "start": 4456520,
   "end": 4529426
  }, {
   "filename": "/Images/pick-ups.c16",
   "start": 4529426,
   "end": 5012646
  }, {
   "filename": "/Images/i40a.c16",
   "start": 5012646,
   "end": 5018320
  }, {
   "filename": "/Images/m10a.c16",
   "start": 5018320,
   "end": 5028482
  }, {
   "filename": "/Images/h05a.c16",
   "start": 5028482,
   "end": 5041222
  }, {
   "filename": "/Images/l55a.c16",
   "start": 5041222,
   "end": 5071028
  }, {
   "filename": "/Images/switch.c16",
   "start": 5071028,
   "end": 5089546
  }, {
   "filename": "/Images/door.c16",
   "start": 5089546,
   "end": 9264916
  }, {
   "filename": "/Images/d05a.c16",
   "start": 9264916,
   "end": 9282208
  }, {
   "filename": "/Images/e40a.c16",
   "start": 9282208,
   "end": 9288454
  }, {
   "filename": "/Images/a10a.c16",
   "start": 9288454,
   "end": 9830272
  }, {
   "filename": "/Images/d52a.c16",
   "start": 9830272,
   "end": 9842634
  }, {
   "filename": "/Images/h44a.c16",
   "start": 9842634,
   "end": 9855374
  }, {
   "filename": "/Images/gnat.c16",
   "start": 9855374,
   "end": 9870048
  }, {
   "filename": "/Images/l14a.c16",
   "start": 9870048,
   "end": 9899854
  }, {
   "filename": "/Images/rotorfly.c16",
   "start": 9899854,
   "end": 10034648
  }, {
   "filename": "/Images/lightmod.c16",
   "start": 10034648,
   "end": 10057490
  }, {
   "filename": "/Images/g00b.c16",
   "start": 10057490,
   "end": 10065724
  }, {
   "filename": "/Images/targ.c16",
   "start": 10065724,
   "end": 10094450
  }, {
   "filename": "/Images/e60a.c16",
   "start": 10094450,
   "end": 10100696
  }, {
   "filename": "/Images/fanplant.c16",
   "start": 10100696,
   "end": 10112314
  }, {
   "filename": "/Images/h44c.c16",
   "start": 10112314,
   "end": 10125054
  }, {
   "filename": "/Images/d25a.c16",
   "start": 10125054,
   "end": 10137416
  }, {
   "filename": "/Images/bridge airlock.c16",
   "start": 10137416,
   "end": 10948118
  }, {
   "filename": "/Images/mantis.c16",
   "start": 10948118,
   "end": 10971594
  }, {
   "filename": "/Images/f45b.c16",
   "start": 10971594,
   "end": 10990020
  }, {
   "filename": "/Images/door_ports.c16",
   "start": 10990020,
   "end": 10991152
  }, {
   "filename": "/Images/j04b.c16",
   "start": 10991152,
   "end": 11006594
  }, {
   "filename": "/Images/h64a.c16",
   "start": 11006594,
   "end": 11015876
  }, {
   "filename": "/Images/e40c.c16",
   "start": 11015876,
   "end": 11022122
  }, {
   "filename": "/Images/b42b.c16",
   "start": 11022122,
   "end": 11163032
  }, {
   "filename": "/Images/d05c.c16",
   "start": 11163032,
   "end": 11180324
  }, {
   "filename": "/Images/euro gui.c16",
   "start": 11180324,
   "end": 11180576
  }, {
   "filename": "/Images/l22a.c16",
   "start": 11180576,
   "end": 11189924
  }, {
   "filename": "/Images/h05c.c16",
   "start": 11189924,
   "end": 11202664
  }, {
   "filename": "/Images/n42b.c16",
   "start": 11202664,
   "end": 11212554
  }, {
   "filename": "/Images/lung.c16",
   "start": 11212554,
   "end": 13506654
  }, {
   "filename": "/Images/d64a.c16",
   "start": 13506654,
   "end": 13519016
  }, {
   "filename": "/Images/i40c.c16",
   "start": 13519016,
   "end": 13524690
  }, {
   "filename": "/Images/f04b.c16",
   "start": 13524690,
   "end": 13543116
  }, {
   "filename": "/Images/crypt door.c16",
   "start": 13543116,
   "end": 13603938
  }, {
   "filename": "/Images/l02c.c16",
   "start": 13603938,
   "end": 13615152
  }, {
   "filename": "/Images/j45b.c16",
   "start": 13615152,
   "end": 13630594
  }, {
   "filename": "/Images/h25a.c16",
   "start": 13630594,
   "end": 13639876
  }, {
   "filename": "/Images/i60a.c16",
   "start": 13639876,
   "end": 13645550
  }, {
   "filename": "/Images/graspit.c16",
   "start": 13645550,
   "end": 13758980
  }, {
   "filename": "/Images/d44c.c16",
   "start": 13758980,
   "end": 13776272
  }, {
   "filename": "/Images/k00b.c16",
   "start": 13776272,
   "end": 13781946
  }, {
   "filename": "/Images/042a.c16",
   "start": 13781946,
   "end": 13843968
  }, {
   "filename": "/Images/f24a.c16",
   "start": 13843968,
   "end": 13857118
  }, {
   "filename": "/Images/k00c.c16",
   "start": 13857118,
   "end": 13862792
  }, {
   "filename": "/Images/d44b.c16",
   "start": 13862792,
   "end": 13880084
  }, {
   "filename": "/Images/dead_fish.c16",
   "start": 13880084,
   "end": 13981828
  }, {
   "filename": "/Images/j45c.c16",
   "start": 13981828,
   "end": 13997270
  }, {
   "filename": "/Images/l02b.c16",
   "start": 13997270,
   "end": 14008484
  }, {
   "filename": "/Images/progress.s16",
   "start": 14008484,
   "end": 14201052
  }, {
   "filename": "/Images/k20a.c16",
   "start": 14201052,
   "end": 14206726
  }, {
   "filename": "/Images/f04c.c16",
   "start": 14206726,
   "end": 14225152
  }, {
   "filename": "/Images/i40b.c16",
   "start": 14225152,
   "end": 14230826
  }, {
   "filename": "/Images/015a.c16",
   "start": 14230826,
   "end": 14308208
  }, {
   "filename": "/Images/n42c.c16",
   "start": 14308208,
   "end": 14318098
  }, {
   "filename": "/Images/h05b.c16",
   "start": 14318098,
   "end": 14330838
  }, {
   "filename": "/Images/j65a.c16",
   "start": 14330838,
   "end": 14342052
  }, {
   "filename": "/Images/trout.c16",
   "start": 14342052,
   "end": 14414352
  }, {
   "filename": "/Images/airlock buttons.c16",
   "start": 14414352,
   "end": 14499688
  }, {
   "filename": "/Images/f65a.c16",
   "start": 14499688,
   "end": 14512838
  }, {
   "filename": "/Images/smell_machines.c16",
   "start": 14512838,
   "end": 14706836
  }, {
   "filename": "/Images/d05b.c16",
   "start": 14706836,
   "end": 14724128
  }, {
   "filename": "/Images/b42c.c16",
   "start": 14724128,
   "end": 14865038
  }, {
   "filename": "/Images/e40b.c16",
   "start": 14865038,
   "end": 14871284
  }, {
   "filename": "/Images/g20a.c16",
   "start": 14871284,
   "end": 14879518
  }, {
   "filename": "/Images/j04c.c16",
   "start": 14879518,
   "end": 14894960
  }, {
   "filename": "/Images/gren.c16",
   "start": 14894960,
   "end": 14898240
  }, {
   "filename": "/Images/mossie.c16",
   "start": 14898240,
   "end": 14922450
  }, {
   "filename": "/Images/clown.c16",
   "start": 14922450,
   "end": 14986402
  }, {
   "filename": "/Images/f45c.c16",
   "start": 14986402,
   "end": 15004828
  }, {
   "filename": "/Images/b62a.c16",
   "start": 15004828,
   "end": 15125454
  }, {
   "filename": "/Images/054a.c16",
   "start": 15125454,
   "end": 15202836
  }, {
   "filename": "/Images/h44b.c16",
   "start": 15202836,
   "end": 15215576
  }, {
   "filename": "/Images/j24a.c16",
   "start": 15215576,
   "end": 15226790
  }, {
   "filename": "/Images/g00c.c16",
   "start": 15226790,
   "end": 15235024
  }, {
   "filename": "/Images/wolf.c16",
   "start": 15235024,
   "end": 15366242
  }, {
   "filename": "/Images/k60a.c16",
   "start": 15366242,
   "end": 15371916
  }, {
   "filename": "/Images/f44c.c16",
   "start": 15371916,
   "end": 15390342
  }, {
   "filename": "/Images/hawk.c16",
   "start": 15390342,
   "end": 15886308
  }, {
   "filename": "/Images/i00b.c16",
   "start": 15886308,
   "end": 15891982
  }, {
   "filename": "/Images/055a.c16",
   "start": 15891982,
   "end": 15969364
  }, {
   "filename": "/Images/n02c.c16",
   "start": 15969364,
   "end": 15977866
  }, {
   "filename": "/Images/h45b.c16",
   "start": 15977866,
   "end": 15990606
  }, {
   "filename": "/Images/j25a.c16",
   "start": 15990606,
   "end": 16001820
  }, {
   "filename": "/Images/002a.c16",
   "start": 16001820,
   "end": 16063842
  }, {
   "filename": "/Images/f64a.c16",
   "start": 16063842,
   "end": 16076992
  }, {
   "filename": "/Images/k40c.c16",
   "start": 16076992,
   "end": 16082666
  }, {
   "filename": "/Images/d04b.c16",
   "start": 16082666,
   "end": 16099958
  }, {
   "filename": "/Images/j05c.c16",
   "start": 16099958,
   "end": 16115400
  }, {
   "filename": "/Images/l42b.c16",
   "start": 16115400,
   "end": 16126614
  }, {
   "filename": "/Images/bubs.c16",
   "start": 16126614,
   "end": 16149568
  }, {
   "filename": "/Images/f05c.c16",
   "start": 16149568,
   "end": 16167994
  }, {
   "filename": "/Images/caos.c16",
   "start": 16167994,
   "end": 16184412
  }, {
   "filename": "/Images/b22a.c16",
   "start": 16184412,
   "end": 16296074
  }, {
   "filename": "/Images/014a.c16",
   "start": 16296074,
   "end": 16373456
  }, {
   "filename": "/Images/fish launcher things.c16",
   "start": 16373456,
   "end": 16423940
  }, {
   "filename": "/Images/h04b.c16",
   "start": 16423940,
   "end": 16436680
  }, {
   "filename": "/Images/j64a.c16",
   "start": 16436680,
   "end": 16447894
  }, {
   "filename": "/Images/g40c.c16",
   "start": 16447894,
   "end": 16456128
  }, {
   "filename": "/Images/f25a.c16",
   "start": 16456128,
   "end": 16469278
  }, {
   "filename": "/Images/d45b.c16",
   "start": 16469278,
   "end": 16486570
  }, {
   "filename": "/Images/b02c.c16",
   "start": 16486570,
   "end": 16614256
  }, {
   "filename": "/Images/e00b.c16",
   "start": 16614256,
   "end": 16620502
  }, {
   "filename": "/Images/g60a.c16",
   "start": 16620502,
   "end": 16628736
  }, {
   "filename": "/Images/j44c.c16",
   "start": 16628736,
   "end": 16644178
  }, {
   "filename": "/Images/j44b.c16",
   "start": 16644178,
   "end": 16659620
  }, {
   "filename": "/Images/h24a.c16",
   "start": 16659620,
   "end": 16668902
  }, {
   "filename": "/Images/e00c.c16",
   "start": 16668902,
   "end": 16675148
  }, {
   "filename": "/Images/b02b.c16",
   "start": 16675148,
   "end": 16802834
  }, {
   "filename": "/Images/d45c.c16",
   "start": 16802834,
   "end": 16820126
  }, {
   "filename": "/Images/beetle2.c16",
   "start": 16820126,
   "end": 16892324
  }, {
   "filename": "/Images/g40b.c16",
   "start": 16892324,
   "end": 16900558
  }, {
   "filename": "/Images/e20a.c16",
   "start": 16900558,
   "end": 16906804
  }, {
   "filename": "/Images/h04c.c16",
   "start": 16906804,
   "end": 16919544
  }, {
   "filename": "/Images/d65a.c16",
   "start": 16919544,
   "end": 16931906
  }, {
   "filename": "/Images/test.c16",
   "start": 16931906,
   "end": 16932878
  }, {
   "filename": "/Images/f05b.c16",
   "start": 16932878,
   "end": 16951304
  }, {
   "filename": "/Images/envi.c16",
   "start": 16951304,
   "end": 17100152
  }, {
   "filename": "/Images/l42c.c16",
   "start": 17100152,
   "end": 17111366
  }, {
   "filename": "/Images/j05b.c16",
   "start": 17111366,
   "end": 17126808
  }, {
   "filename": "/Images/h65a.c16",
   "start": 17126808,
   "end": 17136090
  }, {
   "filename": "/Images/i20a.c16",
   "start": 17136090,
   "end": 17141764
  }, {
   "filename": "/Images/d04c.c16",
   "start": 17141764,
   "end": 17159056
  }, {
   "filename": "/Images/k40b.c16",
   "start": 17159056,
   "end": 17164730
  }, {
   "filename": "/Images/apple.c16",
   "start": 17164730,
   "end": 17173752
  }, {
   "filename": "/Images/aquatic cave water.c16",
   "start": 17173752,
   "end": 21741032
  }, {
   "filename": "/Images/l62a.c16",
   "start": 21741032,
   "end": 21750380
  }, {
   "filename": "/Images/h45c.c16",
   "start": 21750380,
   "end": 21763120
  }, {
   "filename": "/Images/n02b.c16",
   "start": 21763120,
   "end": 21771622
  }, {
   "filename": "/Images/grass.c16",
   "start": 21771622,
   "end": 21796500
  }, {
   "filename": "/Images/graph.c16",
   "start": 21796500,
   "end": 22206766
  }, {
   "filename": "/Images/d24a.c16",
   "start": 22206766,
   "end": 22219128
  }, {
   "filename": "/Images/i00c.c16",
   "start": 22219128,
   "end": 22224802
  }, {
   "filename": "/Images/f44b.c16",
   "start": 22224802,
   "end": 22243228
  }, {
   "filename": "/Images/world switcher.c16",
   "start": 22243228,
   "end": 23789842
  }, {
   "filename": "/Images/splash.c16",
   "start": 23789842,
   "end": 23790874
  }, {
   "filename": "/Images/cameraunit.c16",
   "start": 23790874,
   "end": 24218054
  }, {
   "filename": "/Images/gumin.c16",
   "start": 24218054,
   "end": 24363704
  }, {
   "filename": "/Images/meerk.c16",
   "start": 24363704,
   "end": 24663914
  }, {
   "filename": "/Images/i00a.c16",
   "start": 24663914,
   "end": 24669588
  }, {
   "filename": "/Images/m50a.c16",
   "start": 24669588,
   "end": 24679750
  }, {
   "filename": "/Images/h45a.c16",
   "start": 24679750,
   "end": 24692490
  }, {
   "filename": "/Images/l15a.c16",
   "start": 24692490,
   "end": 24722296
  }, {
   "filename": "/Images/d04a.c16",
   "start": 24722296,
   "end": 24739588
  }, {
   "filename": "/Images/002b.c16",
   "start": 24739588,
   "end": 24801610
  }, {
   "filename": "/Images/l42a.c16",
   "start": 24801610,
   "end": 24812824
  }, {
   "filename": "/Images/h12a.c16",
   "start": 24812824,
   "end": 24827914
  }, {
   "filename": "/Images/d12a.c16",
   "start": 24827914,
   "end": 24840276
  }, {
   "filename": "/Images/h04a.c16",
   "start": 24840276,
   "end": 24853016
  }, {
   "filename": "/Images/l54a.c16",
   "start": 24853016,
   "end": 24882822
  }, {
   "filename": "/Images/d45a.c16",
   "start": 24882822,
   "end": 24900114
  }, {
   "filename": "/Images/aquamites.c16",
   "start": 24900114,
   "end": 24905578
  }, {
   "filename": "/Images/e00a.c16",
   "start": 24905578,
   "end": 24911824
  }, {
   "filename": "/Images/a50a.c16",
   "start": 24911824,
   "end": 25487730
  }, {
   "filename": "/Images/j44a.c16",
   "start": 25487730,
   "end": 25503172
  }, {
   "filename": "/Images/n14a.c16",
   "start": 25503172,
   "end": 25520716
  }, {
   "filename": "/Images/b02a.c16",
   "start": 25520716,
   "end": 25643866
  }, {
   "filename": "/Images/clickhere.c16",
   "start": 25643866,
   "end": 25824608
  }, {
   "filename": "/Images/f52a.c16",
   "start": 25824608,
   "end": 25837758
  }, {
   "filename": "/Images/hatch2.c16",
   "start": 25837758,
   "end": 27722002
  }, {
   "filename": "/Images/pirbubbs.c16",
   "start": 27722002,
   "end": 28181746
  }, {
   "filename": "/Images/g40a.c16",
   "start": 28181746,
   "end": 28189980
  }, {
   "filename": "/Images/c10a.c16",
   "start": 28189980,
   "end": 28198700
  }, {
   "filename": "/Images/f05a.c16",
   "start": 28198700,
   "end": 28217126
  }, {
   "filename": "/Images/b55a.c16",
   "start": 28217126,
   "end": 28605216
  }, {
   "filename": "/Images/euro medicine maker.c16",
   "start": 28605216,
   "end": 28676544
  }, {
   "filename": "/Images/airlock_mid.c16",
   "start": 28676544,
   "end": 28799920
  }, {
   "filename": "/Images/j05a.c16",
   "start": 28799920,
   "end": 28815362
  }, {
   "filename": "/Images/n55a.c16",
   "start": 28815362,
   "end": 28832906
  }, {
   "filename": "/Images/k40a.c16",
   "start": 28832906,
   "end": 28838580
  }, {
   "filename": "/Images/q00a.c16",
   "start": 28838580,
   "end": 28842778
  }, {
   "filename": "/Images/002c.c16",
   "start": 28842778,
   "end": 28904800
  }, {
   "filename": "/Images/n02a.c16",
   "start": 28904800,
   "end": 28913302
  }, {
   "filename": "/Images/recycler overlay.c16",
   "start": 28913302,
   "end": 28917336
  }, {
   "filename": "/Images/j52a.c16",
   "start": 28917336,
   "end": 28938262
  }, {
   "filename": "/Images/choppertoy.c16",
   "start": 28938262,
   "end": 28998136
  }, {
   "filename": "/Images/022a.c16",
   "start": 28998136,
   "end": 29052478
  }, {
   "filename": "/Images/f44a.c16",
   "start": 29052478,
   "end": 29070904
  }, {
   "filename": "/Images/grasshopper.c16",
   "start": 29070904,
   "end": 29130206
  }, {
   "filename": "/Images/b14a.c16",
   "start": 29130206,
   "end": 29524372
  }, {
   "filename": "/Images/death_sludge.c16",
   "start": 29524372,
   "end": 30086320
  }, {
   "filename": "/Images/n05b.c16",
   "start": 30086320,
   "end": 30098232
  }, {
   "filename": "/Images/h42c.c16",
   "start": 30098232,
   "end": 30107514
  }, {
   "filename": "/Images/l65a.c16",
   "start": 30107514,
   "end": 30118728
  }, {
   "filename": "/Images/m20a.c16",
   "start": 30118728,
   "end": 30121134
  }, {
   "filename": "/Images/progress-es.s16",
   "start": 30121134,
   "end": 30315622
  }, {
   "filename": "/Images/disease and injury screen.c16",
   "start": 30315622,
   "end": 31117718
  }, {
   "filename": "/Images/h62a.c16",
   "start": 31117718,
   "end": 31125580
  }, {
   "filename": "/Images/j02b.c16",
   "start": 31125580,
   "end": 31136794
  }, {
   "filename": "/Images/l45c.c16",
   "start": 31136794,
   "end": 31152236
  }, {
   "filename": "/Images/detector.c16",
   "start": 31152236,
   "end": 31234750
  }, {
   "filename": "/Images/m00c.c16",
   "start": 31234750,
   "end": 31239008
  }, {
   "filename": "/Images/b44b.c16",
   "start": 31239008,
   "end": 31446592
  }, {
   "filename": "/Images/gene splicer overlay.c16",
   "start": 31446592,
   "end": 31583714
  }, {
   "filename": "/Images/n44b.c16",
   "start": 31583714,
   "end": 31597824
  }, {
   "filename": "/Images/l24a.c16",
   "start": 31597824,
   "end": 31609038
  }, {
   "filename": "/Images/a00c.c16",
   "start": 31609038,
   "end": 32006192
  }, {
   "filename": "/Images/f02b.c16",
   "start": 32006192,
   "end": 32019342
  }, {
   "filename": "/Images/d62a.c16",
   "start": 32019342,
   "end": 32029534
  }, {
   "filename": "/Images/greneggmask.c16",
   "start": 32029534,
   "end": 32329938
  }, {
   "filename": "/Images/chars.s16",
   "start": 32329938,
   "end": 32347352
  }, {
   "filename": "/Images/marine room cutout.c16",
   "start": 32347352,
   "end": 32358970
  }, {
   "filename": "/Images/c40b.c16",
   "start": 32358970,
   "end": 32367690
  }, {
   "filename": "/Images/a20a.c16",
   "start": 32367690,
   "end": 32694460
  }, {
   "filename": "/Images/l04c.c16",
   "start": 32694460,
   "end": 32709902
  }, {
   "filename": "/Images/d42c.c16",
   "start": 32709902,
   "end": 32722264
  }, {
   "filename": "/Images/b05b.c16",
   "start": 32722264,
   "end": 32908670
  }, {
   "filename": "/Images/b05c.c16",
   "start": 32908670,
   "end": 33095076
  }, {
   "filename": "/Images/d42b.c16",
   "start": 33095076,
   "end": 33107438
  }, {
   "filename": "/Images/f22a.c16",
   "start": 33107438,
   "end": 33118242
  }, {
   "filename": "/Images/044a.c16",
   "start": 33118242,
   "end": 33195624
  }, {
   "filename": "/Images/l04b.c16",
   "start": 33195624,
   "end": 33211066
  }, {
   "filename": "/Images/c40c.c16",
   "start": 33211066,
   "end": 33219786
  }, {
   "filename": "/Images/grabber.c16",
   "start": 33219786,
   "end": 33529872
  }, {
   "filename": "/Images/tendril.c16",
   "start": 33529872,
   "end": 33596798
  }, {
   "filename": "/Images/b25a.c16",
   "start": 33596798,
   "end": 33736700
  }, {
   "filename": "/Images/f02c.c16",
   "start": 33736700,
   "end": 33749850
  }, {
   "filename": "/Images/a00b.c16",
   "start": 33749850,
   "end": 34126864
  }, {
   "filename": "/Images/c60a.c16",
   "start": 34126864,
   "end": 34135584
  }, {
   "filename": "/Images/n44c.c16",
   "start": 34135584,
   "end": 34149694
  }, {
   "filename": "/Images/medipod.c16",
   "start": 34149694,
   "end": 34730854
  }, {
   "filename": "/Images/sidebar.c16",
   "start": 34730854,
   "end": 34856570
  }, {
   "filename": "/Images/b44c.c16",
   "start": 34856570,
   "end": 35064154
  }, {
   "filename": "/Images/005a.c16",
   "start": 35064154,
   "end": 35141536
  }, {
   "filename": "/Images/m00b.c16",
   "start": 35141536,
   "end": 35145794
  }, {
   "filename": "/Images/mediscan.c16",
   "start": 35145794,
   "end": 35172806
  }, {
   "filename": "/Images/l45b.c16",
   "start": 35172806,
   "end": 35188248
  }, {
   "filename": "/Images/j02c.c16",
   "start": 35188248,
   "end": 35199462
  }, {
   "filename": "/Images/b64a.c16",
   "start": 35199462,
   "end": 35351044
  }, {
   "filename": "/Images/052a.c16",
   "start": 35351044,
   "end": 35413066
  }, {
   "filename": "/Images/syringe.c16",
   "start": 35413066,
   "end": 36022072
  }, {
   "filename": "/Images/j22a.c16",
   "start": 36022072,
   "end": 36031420
  }, {
   "filename": "/Images/bal2.c16",
   "start": 36031420,
   "end": 36049910
  }, {
   "filename": "/Images/h42b.c16",
   "start": 36049910,
   "end": 36059192
  }, {
   "filename": "/Images/n05c.c16",
   "start": 36059192,
   "end": 36071104
  }, {
   "filename": "/Images/cuttlefish.c16",
   "start": 36071104,
   "end": 36455294
  }, {
   "filename": "/Images/ettin_doors.c16",
   "start": 36455294,
   "end": 36571858
  }, {
   "filename": "/Images/n05a.c16",
   "start": 36571858,
   "end": 36583770
  }, {
   "filename": "/Images/j55a.c16",
   "start": 36583770,
   "end": 36613700
  }, {
   "filename": "/Images/siren.c16",
   "start": 36613700,
   "end": 36657924
  }, {
   "filename": "/Images/k10a.c16",
   "start": 36657924,
   "end": 36665306
  }, {
   "filename": "/Images/norn platform.c16",
   "start": 36665306,
   "end": 36820894
  }, {
   "filename": "/Images/025a.c16",
   "start": 36820894,
   "end": 36882916
  }, {
   "filename": "/Images/lung overlay.c16",
   "start": 36882916,
   "end": 36936994
  }, {
   "filename": "/Images/j02a.c16",
   "start": 36936994,
   "end": 36948208
  }, {
   "filename": "/Images/n52a.c16",
   "start": 36948208,
   "end": 36960582
  }, {
   "filename": "/Images/b44a.c16",
   "start": 36960582,
   "end": 37157308
  }, {
   "filename": "/Images/f14a.c16",
   "start": 37157308,
   "end": 37175734
  }, {
   "filename": "/Images/neon.c16",
   "start": 37175734,
   "end": 37230164
  }, {
   "filename": "/Images/005c.c16",
   "start": 37230164,
   "end": 37307546
  }, {
   "filename": "/Images/plnt.c16",
   "start": 37307546,
   "end": 37357450
  }, {
   "filename": "/Images/suckerplant.c16",
   "start": 37357450,
   "end": 37988664
  }, {
   "filename": "/Images/n44a.c16",
   "start": 37988664,
   "end": 38002774
  }, {
   "filename": "/Images/j14a.c16",
   "start": 38002774,
   "end": 38032704
  }, {
   "filename": "/Images/f02a.c16",
   "start": 38032704,
   "end": 38045854
  }, {
   "filename": "/Images/064a.c16",
   "start": 38045854,
   "end": 38107876
  }, {
   "filename": "/Images/b52a.c16",
   "start": 38107876,
   "end": 38369526
  }, {
   "filename": "/Images/injection.c16",
   "start": 38369526,
   "end": 38960416
  }, {
   "filename": "/Images/transporter pod.c16",
   "start": 38960416,
   "end": 39242434
  }, {
   "filename": "/Images/c40a.c16",
   "start": 39242434,
   "end": 39251154
  }, {
   "filename": "/Images/g10a.c16",
   "start": 39251154,
   "end": 39259388
  }, {
   "filename": "/Images/b05a.c16",
   "start": 39259388,
   "end": 39438914
  }, {
   "filename": "/Images/f55a.c16",
   "start": 39438914,
   "end": 39457340
  }, {
   "filename": "/Images/044c.c16",
   "start": 39457340,
   "end": 39534722
  }, {
   "filename": "/Images/044b.c16",
   "start": 39534722,
   "end": 39612104
  }, {
   "filename": "/Images/d42a.c16",
   "start": 39612104,
   "end": 39624466
  }, {
   "filename": "/Images/click_here.c16",
   "start": 39624466,
   "end": 39726014
  }, {
   "filename": "/Images/l04a.c16",
   "start": 39726014,
   "end": 39741456
  }, {
   "filename": "/Images/h54a.c16",
   "start": 39741456,
   "end": 39762254
  }, {
   "filename": "/Images/mediporter.c16",
   "start": 39762254,
   "end": 39789282
  }, {
   "filename": "/Images/creature_history.c16",
   "start": 39789282,
   "end": 41458224
  }, {
   "filename": "/Images/button.c16",
   "start": 41458224,
   "end": 41461058
  }, {
   "filename": "/Images/d15a.c16",
   "start": 41461058,
   "end": 41478350
  }, {
   "filename": "/Images/infection detector.c16",
   "start": 41478350,
   "end": 41509344
  }, {
   "filename": "/Images/a00a.c16",
   "start": 41509344,
   "end": 41872584
  }, {
   "filename": "/Images/bluebell.c16",
   "start": 41872584,
   "end": 41901664
  }, {
   "filename": "/Images/e50a.c16",
   "start": 41901664,
   "end": 41911830
  }, {
   "filename": "/Images/snail.c16",
   "start": 41911830,
   "end": 42049960
  }, {
   "filename": "/Images/tank.c16",
   "start": 42049960,
   "end": 42065478
  }, {
   "filename": "/Images/m00a.c16",
   "start": 42065478,
   "end": 42069736
  }, {
   "filename": "/Images/i50a.c16",
   "start": 42069736,
   "end": 42077094
  }, {
   "filename": "/Images/005b.c16",
   "start": 42077094,
   "end": 42154476
  }, {
   "filename": "/Images/gender.c16",
   "start": 42154476,
   "end": 42155602
  }, {
   "filename": "/Images/l45a.c16",
   "start": 42155602,
   "end": 42171044
  }, {
   "filename": "/Images/h15a.c16",
   "start": 42171044,
   "end": 42191842
  }, {
   "filename": "/Images/logicgates.c16",
   "start": 42191842,
   "end": 42197604
  }, {
   "filename": "/Images/d54a.c16",
   "start": 42197604,
   "end": 42214896
  }, {
   "filename": "/Images/kobold.c16",
   "start": 42214896,
   "end": 42647874
  }, {
   "filename": "/Images/h42a.c16",
   "start": 42647874,
   "end": 42657156
  }, {
   "filename": "/Images/l12a.c16",
   "start": 42657156,
   "end": 42677956
  }, {
   "filename": "/Images/hedgehog.c16",
   "start": 42677956,
   "end": 42810166
  }, {
   "filename": "/Images/d55a.c16",
   "start": 42810166,
   "end": 42827458
  }, {
   "filename": "/Images/crypt2.c16",
   "start": 42827458,
   "end": 45191192
  }, {
   "filename": "/Images/butterfly2.c16",
   "start": 45191192,
   "end": 45240966
  }, {
   "filename": "/Images/a40a.c16",
   "start": 45240966,
   "end": 45655418
  }, {
   "filename": "/Images/e10a.c16",
   "start": 45655418,
   "end": 45665584
  }, {
   "filename": "/Images/hhog.c16",
   "start": 45665584,
   "end": 45956688
  }, {
   "filename": "/Images/004b.c16",
   "start": 45956688,
   "end": 46034070
  }, {
   "filename": "/Images/d02a.c16",
   "start": 46034070,
   "end": 46046432
  }, {
   "filename": "/Images/bubblicious.c16",
   "start": 46046432,
   "end": 46049326
  }, {
   "filename": "/Images/woodpigeon.c16",
   "start": 46049326,
   "end": 46274260
  }, {
   "filename": "/Images/l44a.c16",
   "start": 46274260,
   "end": 46289702
  }, {
   "filename": "/Images/h14a.c16",
   "start": 46289702,
   "end": 46310500
  }, {
   "filename": "/Images/creator.c16",
   "start": 46310500,
   "end": 47112074
  }, {
   "filename": "/Images/d14a.c16",
   "start": 47112074,
   "end": 47129366
  }, {
   "filename": "/Images/liftcall.c16",
   "start": 47129366,
   "end": 47161418
  }, {
   "filename": "/Images/h02a.c16",
   "start": 47161418,
   "end": 47170700
  }, {
   "filename": "/Images/l52a.c16",
   "start": 47170700,
   "end": 47191500
  }, {
   "filename": "/Images/env_con.c16",
   "start": 47191500,
   "end": 47496330
  }, {
   "filename": "/Images/palmplant.c16",
   "start": 47496330,
   "end": 47530612
  }, {
   "filename": "/Images/m40a.c16",
   "start": 47530612,
   "end": 47534870
  }, {
   "filename": "/Images/i10a.c16",
   "start": 47534870,
   "end": 47542228
  }, {
   "filename": "/Images/045b.c16",
   "start": 47542228,
   "end": 47619610
  }, {
   "filename": "/Images/l05a.c16",
   "start": 47619610,
   "end": 47635052
  }, {
   "filename": "/Images/h55a.c16",
   "start": 47635052,
   "end": 47655850
  }, {
   "filename": "/Images/j42a.c16",
   "start": 47655850,
   "end": 47667064
  }, {
   "filename": "/Images/n12a.c16",
   "start": 47667064,
   "end": 47679438
  }, {
   "filename": "/Images/airlock agent.c16",
   "start": 47679438,
   "end": 48949856
  }, {
   "filename": "/Images/b04a.c16",
   "start": 48949856,
   "end": 49129382
  }, {
   "filename": "/Images/robot_toy.c16",
   "start": 49129382,
   "end": 49215254
  }, {
   "filename": "/Images/f54a.c16",
   "start": 49215254,
   "end": 49233680
  }, {
   "filename": "/Images/045c.c16",
   "start": 49233680,
   "end": 49311062
  }, {
   "filename": "/Images/creator_remove_button.c16",
   "start": 49311062,
   "end": 49316818
  }, {
   "filename": "/Images/n45a.c16",
   "start": 49316818,
   "end": 49330928
  }, {
   "filename": "/Images/delayer.c16",
   "start": 49330928,
   "end": 49406522
  }, {
   "filename": "/Images/j15a.c16",
   "start": 49406522,
   "end": 49436452
  }, {
   "filename": "/Images/o00a.c16",
   "start": 49436452,
   "end": 49453226
  }, {
   "filename": "/Images/k50a.c16",
   "start": 49453226,
   "end": 49460608
  }, {
   "filename": "/Images/065a.c16",
   "start": 49460608,
   "end": 49522630
  }, {
   "filename": "/Images/c00a.c16",
   "start": 49522630,
   "end": 49531350
  }, {
   "filename": "/Images/aquamite_maker.c16",
   "start": 49531350,
   "end": 49624368
  }, {
   "filename": "/Images/g50a.c16",
   "start": 49624368,
   "end": 49632602
  }, {
   "filename": "/Images/b45a.c16",
   "start": 49632602,
   "end": 49829328
  }, {
   "filename": "/Images/f15a.c16",
   "start": 49829328,
   "end": 49847754
  }, {
   "filename": "/Images/004c.c16",
   "start": 49847754,
   "end": 49925136
  }, {
   "filename": "/Images/n04a.c16",
   "start": 49925136,
   "end": 49937048
  }, {
   "filename": "/Images/norn to grendel door.c16",
   "start": 49937048,
   "end": 50191966
  }, {
   "filename": "/Images/j54a.c16",
   "start": 50191966,
   "end": 50221896
  }, {
   "filename": "/Images/f42a.c16",
   "start": 50221896,
   "end": 50235046
  }, {
   "filename": "/Images/024a.c16",
   "start": 50235046,
   "end": 50297068
  }, {
   "filename": "/Images/b12a.c16",
   "start": 50297068,
   "end": 50553762
  }, {
   "filename": "/Images/sludgegun.c16",
   "start": 50553762,
   "end": 50872098
  }, {
   "filename": "/Images/bone.c16",
   "start": 50872098,
   "end": 50877566
  }, {
   "filename": "/Images/new_agent_help.c16",
   "start": 50877566,
   "end": 51127768
  }, {
   "filename": "/Images/b65a.c16",
   "start": 51127768,
   "end": 51279350
  }, {
   "filename": "/Images/medical_main.c16",
   "start": 51279350,
   "end": 51514158
  }, {
   "filename": "/Images/f42c.c16",
   "start": 51514158,
   "end": 51527308
  }, {
   "filename": "/Images/progress-de.s16",
   "start": 51527308,
   "end": 51719876
  }, {
   "filename": "/Images/butterfly1.c16",
   "start": 51719876,
   "end": 51837692
  }, {
   "filename": "/Images/a40b.c16",
   "start": 51837692,
   "end": 52198710
  }, {
   "filename": "/Images/c20a.c16",
   "start": 52198710,
   "end": 52207430
  }, {
   "filename": "/Images/n04c.c16",
   "start": 52207430,
   "end": 52219342
  }, {
   "filename": "/Images/crypt.c16",
   "start": 52219342,
   "end": 54060572
  }, {
   "filename": "/Images/b45c.c16",
   "start": 54060572,
   "end": 54268156
  }, {
   "filename": "/Images/d02b.c16",
   "start": 54268156,
   "end": 54280518
  }, {
   "filename": "/Images/f62a.c16",
   "start": 54280518,
   "end": 54291322
  }, {
   "filename": "/Images/hover vehicle.c16",
   "start": 54291322,
   "end": 54357308
  }, {
   "filename": "/Images/004a.c16",
   "start": 54357308,
   "end": 54434690
  }, {
   "filename": "/Images/l44b.c16",
   "start": 54434690,
   "end": 54450132
  }, {
   "filename": "/Images/c00c.c16",
   "start": 54450132,
   "end": 54458852
  }, {
   "filename": "/Images/b24a.c16",
   "start": 54458852,
   "end": 54598754
  }, {
   "filename": "/Images/012a.c16",
   "start": 54598754,
   "end": 54660776
  }, {
   "filename": "/Images/j62a.c16",
   "start": 54660776,
   "end": 54670124
  }, {
   "filename": "/Images/h02b.c16",
   "start": 54670124,
   "end": 54679406
  }, {
   "filename": "/Images/n45c.c16",
   "start": 54679406,
   "end": 54693516
  }, {
   "filename": "/Images/coral.c16",
   "start": 54693516,
   "end": 54695686
  }, {
   "filename": "/Images/b04c.c16",
   "start": 54695686,
   "end": 54882092
  }, {
   "filename": "/Images/045a.c16",
   "start": 54882092,
   "end": 54959474
  }, {
   "filename": "/Images/m40b.c16",
   "start": 54959474,
   "end": 54963732
  }, {
   "filename": "/Images/l05b.c16",
   "start": 54963732,
   "end": 54979174
  }, {
   "filename": "/Images/j42c.c16",
   "start": 54979174,
   "end": 54990388
  }, {
   "filename": "/Images/beetle.c16",
   "start": 54990388,
   "end": 55029602
  }, {
   "filename": "/Images/h22a.c16",
   "start": 55029602,
   "end": 55037464
  }, {
   "filename": "/Images/j42b.c16",
   "start": 55037464,
   "end": 55048678
  }, {
   "filename": "/Images/l05c.c16",
   "start": 55048678,
   "end": 55064120
  }, {
   "filename": "/Images/m40c.c16",
   "start": 55064120,
   "end": 55068378
  }, {
   "filename": "/Images/b04b.c16",
   "start": 55068378,
   "end": 55254784
  }, {
   "filename": "/Images/lightbox.c16",
   "start": 55254784,
   "end": 55269062
  }, {
   "filename": "/Images/n45b.c16",
   "start": 55269062,
   "end": 55283172
  }, {
   "filename": "/Images/h02c.c16",
   "start": 55283172,
   "end": 55292454
  }, {
   "filename": "/Images/l25a.c16",
   "start": 55292454,
   "end": 55303668
  }, {
   "filename": "/Images/m60a.c16",
   "start": 55303668,
   "end": 55306130
  }, {
   "filename": "/Images/bacteria.c16",
   "start": 55306130,
   "end": 55306214
  }, {
   "filename": "/Images/hand.s16",
   "start": 55306214,
   "end": 55401324
  }, {
   "filename": "/Images/c00b.c16",
   "start": 55401324,
   "end": 55410044
  }, {
   "filename": "/Images/zzzz.c16",
   "start": 55410044,
   "end": 55425526
  }, {
   "filename": "/Images/a60a.c16",
   "start": 55425526,
   "end": 55757802
  }, {
   "filename": "/Images/l44c.c16",
   "start": 55757802,
   "end": 55773244
  }, {
   "filename": "/Images/euro welcome screen.c16",
   "start": 55773244,
   "end": 57792450
  }, {
   "filename": "/Images/d02c.c16",
   "start": 57792450,
   "end": 57804812
  }, {
   "filename": "/Images/b45b.c16",
   "start": 57804812,
   "end": 58012396
  }, {
   "filename": "/Images/uglee.c16",
   "start": 58012396,
   "end": 58418890
  }, {
   "filename": "/Images/n04b.c16",
   "start": 58418890,
   "end": 58430802
  }, {
   "filename": "/Images/l64a.c16",
   "start": 58430802,
   "end": 58442016
  }, {
   "filename": "/Images/a40c.c16",
   "start": 58442016,
   "end": 58834858
  }, {
   "filename": "/Images/f42b.c16",
   "start": 58834858,
   "end": 58848008
  }, {
   "filename": "/Images/d22a.c16",
   "start": 58848008,
   "end": 58858200
  }, {
   "filename": "/Images/flutterby.c16",
   "start": 58858200,
   "end": 58923756
  }, {
   "filename": "/Images/progress-it.s16",
   "start": 58923756,
   "end": 59116324
  }, {
   "filename": "/Images/launcher.c16",
   "start": 59116324,
   "end": 59233948
  }, {
   "filename": "/Images/gene splicer panel.c16",
   "start": 59233948,
   "end": 59806724
  }, {
   "filename": "/Images/ball.c16",
   "start": 59806724,
   "end": 59846106
  }, {
   "filename": "/Images/h40b.c16",
   "start": 59846106,
   "end": 59852352
  }, {
   "filename": "/Images/j20a.c16",
   "start": 59852352,
   "end": 59859920
  }, {
   "filename": "/Images/g04c.c16",
   "start": 59859920,
   "end": 59877212
  }, {
   "filename": "/Images/k65a.c16",
   "start": 59877212,
   "end": 59885496
  }, {
   "filename": "/Images/nornatmos.c16",
   "start": 59885496,
   "end": 60735e3
  }, {
   "filename": "/Images/050a.c16",
   "start": 60735e3,
   "end": 60781662
  }, {
   "filename": "/Images/i05b.c16",
   "start": 60781662,
   "end": 60792972
  }, {
   "filename": "/Images/e44b.c16",
   "start": 60792972,
   "end": 60805712
  }, {
   "filename": "/Images/g24a.c16",
   "start": 60805712,
   "end": 60818074
  }, {
   "filename": "/Images/wasp.c16",
   "start": 60818074,
   "end": 60840172
  }, {
   "filename": "/Images/j00c.c16",
   "start": 60840172,
   "end": 60847740
  }, {
   "filename": "/Images/k45c.c16",
   "start": 60847740,
   "end": 60859050
  }, {
   "filename": "/Images/m02b.c16",
   "start": 60859050,
   "end": 60865024
  }, {
   "filename": "/Images/infobar.c16",
   "start": 60865024,
   "end": 60897120
  }, {
   "filename": "/Images/teleport.c16",
   "start": 60897120,
   "end": 61422526
  }, {
   "filename": "/Images/c62a.c16",
   "start": 61422526,
   "end": 61433372
  }, {
   "filename": "/Images/a02b.c16",
   "start": 61433372,
   "end": 62038972
  }, {
   "filename": "/Images/g45c.c16",
   "start": 62038972,
   "end": 62056264
  }, {
   "filename": "/Images/recycler2.c16",
   "start": 62056264,
   "end": 62549892
  }, {
   "filename": "/Images/k24a.c16",
   "start": 62549892,
   "end": 62558176
  }, {
   "filename": "/Images/pregnancy indication device.c16",
   "start": 62558176,
   "end": 62779660
  }, {
   "filename": "/Images/f00c.c16",
   "start": 62779660,
   "end": 62788410
  }, {
   "filename": "/Images/i44b.c16",
   "start": 62788410,
   "end": 62799720
  }, {
   "filename": "/Images/medigraph.c16",
   "start": 62799720,
   "end": 63160386
  }, {
   "filename": "/Images/pumperspikel.c16",
   "start": 63160386,
   "end": 63230954
  }, {
   "filename": "/Images/knowledge.c16",
   "start": 63230954,
   "end": 63374356
  }, {
   "filename": "/Images/e05b.c16",
   "start": 63374356,
   "end": 63387096
  }, {
   "filename": "/Images/c42c.c16",
   "start": 63387096,
   "end": 63400320
  }, {
   "filename": "/Images/g65a.c16",
   "start": 63400320,
   "end": 63412682
  }, {
   "filename": "/Images/nidoor.c16",
   "start": 63412682,
   "end": 63436946
  }, {
   "filename": "/Images/f20a.c16",
   "start": 63436946,
   "end": 63445696
  }, {
   "filename": "/Images/k04c.c16",
   "start": 63445696,
   "end": 63457006
  }, {
   "filename": "/Images/d40b.c16",
   "start": 63457006,
   "end": 63465240
  }, {
   "filename": "/Images/medtextbox.c16",
   "start": 63465240,
   "end": 63517438
  }, {
   "filename": "/Images/i64a.c16",
   "start": 63517438,
   "end": 63525722
  }, {
   "filename": "/Images/d40c.c16",
   "start": 63525722,
   "end": 63533956
  }, {
   "filename": "/Images/k04b.c16",
   "start": 63533956,
   "end": 63545266
  }, {
   "filename": "/Images/a22a.c16",
   "start": 63545266,
   "end": 63962310
  }, {
   "filename": "/Images/c42b.c16",
   "start": 63962310,
   "end": 63975534
  }, {
   "filename": "/Images/e05c.c16",
   "start": 63975534,
   "end": 63988274
  }, {
   "filename": "/Images/plusminus.c16",
   "start": 63988274,
   "end": 64004590
  }, {
   "filename": "/Images/d60a.c16",
   "start": 64004590,
   "end": 64012824
  }, {
   "filename": "/Images/heatherontransparentchars.s16",
   "start": 64012824,
   "end": 64045196
  }, {
   "filename": "/Images/i44c.c16",
   "start": 64045196,
   "end": 64056506
  }, {
   "filename": "/Images/f00b.c16",
   "start": 64056506,
   "end": 64065256
  }, {
   "filename": "/Images/g45b.c16",
   "start": 64065256,
   "end": 64082548
  }, {
   "filename": "/Images/a02c.c16",
   "start": 64082548,
   "end": 64685184
  }, {
   "filename": "/Images/e25a.c16",
   "start": 64685184,
   "end": 64694466
  }, {
   "filename": "/Images/i25a.c16",
   "start": 64694466,
   "end": 64702750
  }, {
   "filename": "/Images/pick-ups2.c16",
   "start": 64702750,
   "end": 65186202
  }, {
   "filename": "/Images/fxgl.c16",
   "start": 65186202,
   "end": 65236058
  }, {
   "filename": "/Images/m02c.c16",
   "start": 65236058,
   "end": 65242032
  }, {
   "filename": "/Images/k45b.c16",
   "start": 65242032,
   "end": 65253342
  }, {
   "filename": "/Images/medical_control_panel_buttons.c16",
   "start": 65253342,
   "end": 65323670
  }, {
   "filename": "/Images/j00b.c16",
   "start": 65323670,
   "end": 65331238
  }, {
   "filename": "/Images/h60a.c16",
   "start": 65331238,
   "end": 65337484
  }, {
   "filename": "/Images/e44c.c16",
   "start": 65337484,
   "end": 65350224
  }, {
   "filename": "/Images/attention.c16",
   "start": 65350224,
   "end": 65354448
  }, {
   "filename": "/Images/i05c.c16",
   "start": 65354448,
   "end": 65365758
  }, {
   "filename": "/Images/m22a.c16",
   "start": 65365758,
   "end": 65368548
  }, {
   "filename": "/Images/g04b.c16",
   "start": 65368548,
   "end": 65385840
  }, {
   "filename": "/Images/e64a.c16",
   "start": 65385840,
   "end": 65395122
  }, {
   "filename": "/Images/h40c.c16",
   "start": 65395122,
   "end": 65401368
  }, {
   "filename": "/Images/drive_images.c16",
   "start": 65401368,
   "end": 65472796
  }, {
   "filename": "/Images/h40a.c16",
   "start": 65472796,
   "end": 65479042
  }, {
   "filename": "/Images/l10a.c16",
   "start": 65479042,
   "end": 65492540
  }, {
   "filename": "/Images/i05a.c16",
   "start": 65492540,
   "end": 65503850
  }, {
   "filename": "/Images/m55a.c16",
   "start": 65503850,
   "end": 65525628
  }, {
   "filename": "/Images/graz.c16",
   "start": 65525628,
   "end": 66393572
  }, {
   "filename": "/Images/engscen.c16",
   "start": 66393572,
   "end": 66402800
  }, {
   "filename": "/Images/ettinseedbank.c16",
   "start": 66402800,
   "end": 66451902
  }, {
   "filename": "/Images/e44a.c16",
   "start": 66451902,
   "end": 66464642
  }, {
   "filename": "/Images/a14a.c16",
   "start": 66464642,
   "end": 67753152
  }, {
   "filename": "/Images/m02a.c16",
   "start": 67753152,
   "end": 67759126
  }, {
   "filename": "/Images/i52a.c16",
   "start": 67759126,
   "end": 67770438
  }, {
   "filename": "/Images/numeric output tool.c16",
   "start": 67770438,
   "end": 67788170
  }, {
   "filename": "/Images/cmsh.c16",
   "start": 67788170,
   "end": 67834690
  }, {
   "filename": "/Images/aquatic_population_monitoring_device.c16",
   "start": 67834690,
   "end": 68004042
  }, {
   "filename": "/Images/a02a.c16",
   "start": 68004042,
   "end": 68583228
  }, {
   "filename": "/Images/e52a.c16",
   "start": 68583228,
   "end": 68598302
  }, {
   "filename": "/Images/i44a.c16",
   "start": 68598302,
   "end": 68609612
  }, {
   "filename": "/Images/m14a.c16",
   "start": 68609612,
   "end": 68631390
  }, {
   "filename": "/Images/indicator.c16",
   "start": 68631390,
   "end": 68639630
  }, {
   "filename": "/Images/dragonfly.c16",
   "start": 68639630,
   "end": 68711188
  }, {
   "filename": "/Images/e05a.c16",
   "start": 68711188,
   "end": 68723928
  }, {
   "filename": "/Images/a55a.c16",
   "start": 68723928,
   "end": 70106918
  }, {
   "filename": "/Images/gene pod.c16",
   "start": 70106918,
   "end": 72132958
  }, {
   "filename": "/Images/d40a.c16",
   "start": 72132958,
   "end": 72141192
  }, {
   "filename": "/Images/k04a.c16",
   "start": 72141192,
   "end": 72152502
  }, {
   "filename": "/Images/waterplant.c16",
   "start": 72152502,
   "end": 72293478
  }, {
   "filename": "/Images/c42a.c16",
   "start": 72293478,
   "end": 72306702
  }, {
   "filename": "/Images/g12a.c16",
   "start": 72306702,
   "end": 72319064
  }, {
   "filename": "/Images/sli4.c16",
   "start": 72319064,
   "end": 72333920
  }, {
   "filename": "/Images/smalltextbox.c16",
   "start": 72333920,
   "end": 72347458
  }, {
   "filename": "/Images/fishbowl.c16",
   "start": 72347458,
   "end": 72368518
  }, {
   "filename": "/Images/f00a.c16",
   "start": 72368518,
   "end": 72377268
  }, {
   "filename": "/Images/b50a.c16",
   "start": 72377268,
   "end": 72537142
  }, {
   "filename": "/Images/norn seed launcher.c16",
   "start": 72537142,
   "end": 73112772
  }, {
   "filename": "/Images/g45a.c16",
   "start": 73112772,
   "end": 73130064
  }, {
   "filename": "/Images/c15a.c16",
   "start": 73130064,
   "end": 73148586
  }, {
   "filename": "/Images/nudibranch.c16",
   "start": 73148586,
   "end": 73343372
  }, {
   "filename": "/Images/rocklice.c16",
   "start": 73343372,
   "end": 73532418
  }, {
   "filename": "/Images/k45a.c16",
   "start": 73532418,
   "end": 73543728
  }, {
   "filename": "/Images/j00a.c16",
   "start": 73543728,
   "end": 73551296
  }, {
   "filename": "/Images/n50a.c16",
   "start": 73551296,
   "end": 73559772
  }, {
   "filename": "/Images/algae.c16",
   "start": 73559772,
   "end": 73571008
  }, {
   "filename": "/Images/beatbox.c16",
   "start": 73571008,
   "end": 73644834
  }, {
   "filename": "/Images/opalsponge.c16",
   "start": 73644834,
   "end": 73838558
  }, {
   "filename": "/Images/agent_help.c16",
   "start": 73838558,
   "end": 74359502
  }, {
   "filename": "/Images/ettn.c16",
   "start": 74359502,
   "end": 74362770
  }, {
   "filename": "/Images/vents.c16",
   "start": 74362770,
   "end": 75340784
  }, {
   "filename": "/Images/k12a.c16",
   "start": 75340784,
   "end": 75352120
  }, {
   "filename": "/Images/g04a.c16",
   "start": 75352120,
   "end": 75369412
  }, {
   "filename": "/Images/c54a.c16",
   "start": 75369412,
   "end": 75387934
  }, {
   "filename": "/Images/rockcut1.c16",
   "start": 75387934,
   "end": 75411986
  }, {
   "filename": "/Images/f40a.c16",
   "start": 75411986,
   "end": 75420736
  }, {
   "filename": "/Images/b10a.c16",
   "start": 75420736,
   "end": 75574510
  }, {
   "filename": "/Images/g05a.c16",
   "start": 75574510,
   "end": 75591802
  }, {
   "filename": "/Images/c55a.c16",
   "start": 75591802,
   "end": 75610324
  }, {
   "filename": "/Images/porc.c16",
   "start": 75610324,
   "end": 75726678
  }, {
   "filename": "/Images/progress-ru.s16",
   "start": 75726678,
   "end": 75919246
  }, {
   "filename": "/Images/k44a.c16",
   "start": 75919246,
   "end": 75930556
  }, {
   "filename": "/Images/c02a.c16",
   "start": 75930556,
   "end": 75943780
  }, {
   "filename": "/Images/g52a.c16",
   "start": 75943780,
   "end": 75956142
  }, {
   "filename": "/Images/lift(old).c16",
   "start": 75956142,
   "end": 76252290
  }, {
   "filename": "/Images/grendeleggbank.c16",
   "start": 76252290,
   "end": 76346434
  }, {
   "filename": "/Images/k52a.c16",
   "start": 76346434,
   "end": 76357770
  }, {
   "filename": "/Images/g44a.c16",
   "start": 76357770,
   "end": 76375062
  }, {
   "filename": "/Images/c14a.c16",
   "start": 76375062,
   "end": 76393584
  }, {
   "filename": "/Images/robin.c16",
   "start": 76393584,
   "end": 76608138
  }, {
   "filename": "/Images/king.c16",
   "start": 76608138,
   "end": 76792090
  }, {
   "filename": "/Images/k05a.c16",
   "start": 76792090,
   "end": 76803400
  }, {
   "filename": "/Images/fungi.c16",
   "start": 76803400,
   "end": 76899736
  }, {
   "filename": "/Images/grenscen.c16",
   "start": 76899736,
   "end": 77195108
  }, {
   "filename": "/Images/j40a.c16",
   "start": 77195108,
   "end": 77202676
  }, {
   "filename": "/Images/p00a.c16",
   "start": 77202676,
   "end": 77219450
  }, {
   "filename": "/Images/n10a.c16",
   "start": 77219450,
   "end": 77227926
  }, {
   "filename": "/Images/rabbit.c16",
   "start": 77227926,
   "end": 77273444
  }, {
   "filename": "/Images/e04a.c16",
   "start": 77273444,
   "end": 77286184
  }, {
   "filename": "/Images/a54a.c16",
   "start": 77286184,
   "end": 78669174
  }, {
   "filename": "/Images/carr.c16",
   "start": 78669174,
   "end": 78684734
  }, {
   "filename": "/Images/m42a.c16",
   "start": 78684734,
   "end": 78690708
  }, {
   "filename": "/Images/i12a.c16",
   "start": 78690708,
   "end": 78702020
  }, {
   "filename": "/Images/favouriteplaces.c16",
   "start": 78702020,
   "end": 78876388
  }, {
   "filename": "/Images/crypt2 patch.c16",
   "start": 78876388,
   "end": 78876688
  }, {
   "filename": "/Images/h00a.c16",
   "start": 78876688,
   "end": 78882934
  }, {
   "filename": "/Images/l50a.c16",
   "start": 78882934,
   "end": 78896432
  }, {
   "filename": "/Images/i45a.c16",
   "start": 78896432,
   "end": 78907742
  }, {
   "filename": "/Images/m15a.c16",
   "start": 78907742,
   "end": 78929520
  }, {
   "filename": "/Images/trapper.c16",
   "start": 78929520,
   "end": 79047610
  }, {
   "filename": "/Images/blackonwhitechars.s16",
   "start": 79047610,
   "end": 79061124
  }, {
   "filename": "/Images/e45a.c16",
   "start": 79061124,
   "end": 79073864
  }, {
   "filename": "/Images/a15a.c16",
   "start": 79073864,
   "end": 80362374
  }, {
   "filename": "/Images/d00a.c16",
   "start": 80362374,
   "end": 80370608
  }, {
   "filename": "/Images/cursor.c16",
   "start": 80370608,
   "end": 80372702
  }, {
   "filename": "/Images/a42a.c16",
   "start": 80372702,
   "end": 81038396
  }, {
   "filename": "/Images/e12a.c16",
   "start": 81038396,
   "end": 81053470
  }, {
   "filename": "/Images/i04a.c16",
   "start": 81053470,
   "end": 81064780
  }, {
   "filename": "/Images/m54a.c16",
   "start": 81064780,
   "end": 81086558
  }, {
   "filename": "/Images/d20a.c16",
   "start": 81086558,
   "end": 81094792
  }, {
   "filename": "/Images/i04c.c16",
   "start": 81094792,
   "end": 81106102
  }, {
   "filename": "/Images/launcher nozzle.c16",
   "start": 81106102,
   "end": 81172230
  }, {
   "filename": "/Images/f40b.c16",
   "start": 81172230,
   "end": 81180980
  }, {
   "filename": "/Images/g05b.c16",
   "start": 81180980,
   "end": 81198272
  }, {
   "filename": "/Images/a42c.c16",
   "start": 81198272,
   "end": 81837004
  }, {
   "filename": "/Images/e65a.c16",
   "start": 81837004,
   "end": 81846286
  }, {
   "filename": "/Images/redhot.c16",
   "start": 81846286,
   "end": 81900424
  }, {
   "filename": "/Images/i24a.c16",
   "start": 81900424,
   "end": 81908708
  }, {
   "filename": "/Images/d00c.c16",
   "start": 81908708,
   "end": 81916942
  }, {
   "filename": "/Images/k44b.c16",
   "start": 81916942,
   "end": 81928252
  }, {
   "filename": "/Images/a62a.c16",
   "start": 81928252,
   "end": 82353400
  }, {
   "filename": "/Images/c02b.c16",
   "start": 82353400,
   "end": 82366624
  }, {
   "filename": "/Images/e45c.c16",
   "start": 82366624,
   "end": 82379364
  }, {
   "filename": "/Images/aquatic tunnel gates.c16",
   "start": 82379364,
   "end": 82628778
  }, {
   "filename": "/Images/i45c.c16",
   "start": 82628778,
   "end": 82640088
  }, {
   "filename": "/Images/m62a.c16",
   "start": 82640088,
   "end": 82642878
  }, {
   "filename": "/Images/medicine maker.c16",
   "start": 82642878,
   "end": 82708028
  }, {
   "filename": "/Images/g44b.c16",
   "start": 82708028,
   "end": 82725320
  }, {
   "filename": "/Images/e24a.c16",
   "start": 82725320,
   "end": 82734602
  }, {
   "filename": "/Images/plant.c16",
   "start": 82734602,
   "end": 82741792
  }, {
   "filename": "/Images/h00c.c16",
   "start": 82741792,
   "end": 82748038
  }, {
   "filename": "/Images/euro scgg.c16",
   "start": 82748038,
   "end": 82938994
  }, {
   "filename": "/Images/bioenergy.c16",
   "start": 82938994,
   "end": 82957624
  }, {
   "filename": "/Images/i65a.c16",
   "start": 82957624,
   "end": 82965908
  }, {
   "filename": "/Images/m42c.c16",
   "start": 82965908,
   "end": 82971882
  }, {
   "filename": "/Images/k05b.c16",
   "start": 82971882,
   "end": 82983192
  }, {
   "filename": "/Images/j40b.c16",
   "start": 82983192,
   "end": 82990760
  }, {
   "filename": "/Images/h20a.c16",
   "start": 82990760,
   "end": 82997006
  }, {
   "filename": "/Images/e04c.c16",
   "start": 82997006,
   "end": 83009746
  }, {
   "filename": "/Images/e04b.c16",
   "start": 83009746,
   "end": 83022486
  }, {
   "filename": "/Images/g64a.c16",
   "start": 83022486,
   "end": 83034848
  }, {
   "filename": "/Images/j40c.c16",
   "start": 83034848,
   "end": 83042416
  }, {
   "filename": "/Images/surface.c16",
   "start": 83042416,
   "end": 84330238
  }, {
   "filename": "/Images/k05c.c16",
   "start": 84330238,
   "end": 84341548
  }, {
   "filename": "/Images/m42b.c16",
   "start": 84341548,
   "end": 84347522
  }, {
   "filename": "/Images/whiteontransparentpasswordchars.s16",
   "start": 84347522,
   "end": 84359816
  }, {
   "filename": "/Images/h00b.c16",
   "start": 84359816,
   "end": 84366062
  }, {
   "filename": "/Images/j60a.c16",
   "start": 84366062,
   "end": 84373630
  }, {
   "filename": "/Images/g44c.c16",
   "start": 84373630,
   "end": 84390922
  }, {
   "filename": "/Images/k25a.c16",
   "start": 84390922,
   "end": 84399206
  }, {
   "filename": "/Images/010a.c16",
   "start": 84399206,
   "end": 84445868
  }, {
   "filename": "/Images/i45b.c16",
   "start": 84445868,
   "end": 84457178
  }, {
   "filename": "/Images/e45b.c16",
   "start": 84457178,
   "end": 84469918
  }, {
   "filename": "/Images/c02c.c16",
   "start": 84469918,
   "end": 84483142
  }, {
   "filename": "/Images/g25a.c16",
   "start": 84483142,
   "end": 84495504
  }, {
   "filename": "/Images/contactsensor.c16",
   "start": 84495504,
   "end": 84527794
  }, {
   "filename": "/Images/f60a.c16",
   "start": 84527794,
   "end": 84536544
  }, {
   "filename": "/Images/gui1.c16",
   "start": 84536544,
   "end": 84950350
  }, {
   "filename": "/Images/k44c.c16",
   "start": 84950350,
   "end": 84961660
  }, {
   "filename": "/Images/d00b.c16",
   "start": 84961660,
   "end": 84969894
  }, {
   "filename": "/Images/c22a.c16",
   "start": 84969894,
   "end": 84980740
  }, {
   "filename": "/Images/a42b.c16",
   "start": 84980740,
   "end": 85567600
  }, {
   "filename": "/Images/g05c.c16",
   "start": 85567600,
   "end": 85584892
  }, {
   "filename": "/Images/k64a.c16",
   "start": 85584892,
   "end": 85593176
  }, {
   "filename": "/Images/gnarl.c16",
   "start": 85593176,
   "end": 86268098
  }, {
   "filename": "/Images/f40c.c16",
   "start": 86268098,
   "end": 86276848
  }, {
   "filename": "/Images/progress-fr.s16",
   "start": 86276848,
   "end": 86469416
  }, {
   "filename": "/Images/i04b.c16",
   "start": 86469416,
   "end": 86480726
  }, {
   "filename": "/Images/radio.c16",
   "start": 86480726,
   "end": 86502616
  }, {
   "filename": "/Images/blackontransparentchars.s16",
   "start": 86502616,
   "end": 86534988
  }, {
   "filename": "/Images/loo_.c16",
   "start": 86534988,
   "end": 86541200
  }, {
   "filename": "/Images/a45a.c16",
   "start": 86541200,
   "end": 87518098
  }, {
   "filename": "/Images/e15a.c16",
   "start": 87518098,
   "end": 87538904
  }, {
   "filename": "/Images/d50a.c16",
   "start": 87538904,
   "end": 87547138
  }, {
   "filename": "/Images/e42a.c16",
   "start": 87547138,
   "end": 87556420
  }, {
   "filename": "/Images/a12a.c16",
   "start": 87556420,
   "end": 88432984
  }, {
   "filename": "/Images/m04a.c16",
   "start": 88432984,
   "end": 88440774
  }, {
   "filename": "/Images/i54a.c16",
   "start": 88440774,
   "end": 88456818
  }, {
   "filename": "/Images/graphing.c16",
   "start": 88456818,
   "end": 89319926
  }, {
   "filename": "/Images/a04a.c16",
   "start": 89319926,
   "end": 90164856
  }, {
   "filename": "/Images/learning machine text part.c16",
   "start": 90164856,
   "end": 90178388
  }, {
   "filename": "/Images/e54a.c16",
   "start": 90178388,
   "end": 90199194
  }, {
   "filename": "/Images/i42a.c16",
   "start": 90199194,
   "end": 90207478
  }, {
   "filename": "/Images/m12a.c16",
   "start": 90207478,
   "end": 90222902
  }, {
   "filename": "/Images/spacetoy.c16",
   "start": 90222902,
   "end": 90247742
  }, {
   "filename": "/Images/l00a.c16",
   "start": 90247742,
   "end": 90255310
  }, {
   "filename": "/Images/h50a.c16",
   "start": 90255310,
   "end": 90265468
  }, {
   "filename": "/Images/m45a.c16",
   "start": 90265468,
   "end": 90273258
  }, {
   "filename": "/Images/infinite_cheese_machine.c16",
   "start": 90273258,
   "end": 91246400
  }, {
   "filename": "/Images/040b.c16",
   "start": 91246400,
   "end": 91293062
  }, {
   "filename": "/Images/i15a.c16",
   "start": 91293062,
   "end": 91309106
  }, {
   "filename": "/Images/desertgrass.c16",
   "start": 91309106,
   "end": 91329936
  }, {
   "filename": "/Images/k02a.c16",
   "start": 91329936,
   "end": 91338220
  }, {
   "filename": "/Images/040c.c16",
   "start": 91338220,
   "end": 91384882
  }, {
   "filename": "/Images/rock.c16",
   "start": 91384882,
   "end": 91392218
  }, {
   "filename": "/Images/c44a.c16",
   "start": 91392218,
   "end": 91410740
  }, {
   "filename": "/Images/g14a.c16",
   "start": 91410740,
   "end": 91428032
  }, {
   "filename": "/Images/anti-infection spray.c16",
   "start": 91428032,
   "end": 91465970
  }, {
   "filename": "/Images/injection text part.c16",
   "start": 91465970,
   "end": 91482222
  }, {
   "filename": "/Images/cacbana.c16",
   "start": 91482222,
   "end": 91673154
  }, {
   "filename": "/Images/volcano.c16",
   "start": 91673154,
   "end": 100425300
  }, {
   "filename": "/Images/speech_bubbles.c16",
   "start": 100425300,
   "end": 100546350
  }, {
   "filename": "/Images/k55a.c16",
   "start": 100546350,
   "end": 100562372
  }, {
   "filename": "/Images/060a.c16",
   "start": 100562372,
   "end": 100609034
  }, {
   "filename": "/Images/n40a.c16",
   "start": 100609034,
   "end": 100615772
  }, {
   "filename": "/Images/j10a.c16",
   "start": 100615772,
   "end": 100629578
  }, {
   "filename": "/Images/blnk.c16",
   "start": 100629578,
   "end": 100631196
  }, {
   "filename": "/Images/b40a.c16",
   "start": 100631196,
   "end": 100714094
  }, {
   "filename": "/Images/balloonbug.c16",
   "start": 100714094,
   "end": 100743324
  }, {
   "filename": "/Images/f10a.c16",
   "start": 100743324,
   "end": 100752074
  }, {
   "filename": "/Images/c05a.c16",
   "start": 100752074,
   "end": 100770596
  }, {
   "filename": "/Images/g55a.c16",
   "start": 100770596,
   "end": 100787888
  }, {
   "filename": "/Images/ant.c16",
   "start": 100787888,
   "end": 100861226
  }, {
   "filename": "/Images/single chemical graphing gadget.c16",
   "start": 100861226,
   "end": 101010704
  }, {
   "filename": "/Images/k14a.c16",
   "start": 101010704,
   "end": 101026726
  }, {
   "filename": "/Images/g02a.c16",
   "start": 101026726,
   "end": 101039088
  }, {
   "filename": "/Images/c52a.c16",
   "start": 101039088,
   "end": 101052312
  }, {
   "filename": "/Images/engineering airlock.c16",
   "start": 101052312,
   "end": 101663414
  }, {
   "filename": "/Images/g02c.c16",
   "start": 101663414,
   "end": 101675776
  }, {
   "filename": "/Images/a45b.c16",
   "start": 101675776,
   "end": 102590866
  }, {
   "filename": "/Images/c25a.c16",
   "start": 102590866,
   "end": 102604090
  }, {
   "filename": "/Images/b60a.c16",
   "start": 102604090,
   "end": 102697028
  }, {
   "filename": "/Images/g22a.c16",
   "start": 102697028,
   "end": 102707220
  }, {
   "filename": "/Images/bongo_drums.c16",
   "start": 102707220,
   "end": 102769828
  }, {
   "filename": "/Images/c05c.c16",
   "start": 102769828,
   "end": 102788350
  }, {
   "filename": "/Images/e42b.c16",
   "start": 102788350,
   "end": 102797632
  }, {
   "filename": "/Images/b40c.c16",
   "start": 102797632,
   "end": 102884358
  }, {
   "filename": "/Images/m04b.c16",
   "start": 102884358,
   "end": 102892148
  }, {
   "filename": "/Images/a04b.c16",
   "start": 102892148,
   "end": 103794486
  }, {
   "filename": "/Images/c64a.c16",
   "start": 103794486,
   "end": 103807710
  }, {
   "filename": "/Images/n40c.c16",
   "start": 103807710,
   "end": 103814448
  }, {
   "filename": "/Images/andtheworldexplodedintostars.c16",
   "start": 103814448,
   "end": 103834958
  }, {
   "filename": "/Images/i42b.c16",
   "start": 103834958,
   "end": 103843242
  }, {
   "filename": "/Images/k22a.c16",
   "start": 103843242,
   "end": 103849952
  }, {
   "filename": "/Images/rabb.c16",
   "start": 103849952,
   "end": 103893716
  }, {
   "filename": "/Images/eggs.c16",
   "start": 103893716,
   "end": 104420170
  }, {
   "filename": "/Images/lift.c16",
   "start": 104420170,
   "end": 104738348
  }, {
   "filename": "/Images/l00b.c16",
   "start": 104738348,
   "end": 104745916
  }, {
   "filename": "/Images/c44c.c16",
   "start": 104745916,
   "end": 104764438
  }, {
   "filename": "/Images/mapdisplay.c16",
   "start": 104764438,
   "end": 107338906
  }, {
   "filename": "/Images/040a.c16",
   "start": 107338906,
   "end": 107385568
  }, {
   "filename": "/Images/m45b.c16",
   "start": 107385568,
   "end": 107393358
  }, {
   "filename": "/Images/snotrock.c16",
   "start": 107393358,
   "end": 107468486
  }, {
   "filename": "/Images/k02c.c16",
   "start": 107468486,
   "end": 107476770
  }, {
   "filename": "/Images/k02b.c16",
   "start": 107476770,
   "end": 107485054
  }, {
   "filename": "/Images/m45c.c16",
   "start": 107485054,
   "end": 107492844
  }, {
   "filename": "/Images/i62a.c16",
   "start": 107492844,
   "end": 107499554
  }, {
   "filename": "/Images/bee.c16",
   "start": 107499554,
   "end": 107544416
  }, {
   "filename": "/Images/c44b.c16",
   "start": 107544416,
   "end": 107562938
  }, {
   "filename": "/Images/a24a.c16",
   "start": 107562938,
   "end": 108082770
  }, {
   "filename": "/Images/l00c.c16",
   "start": 108082770,
   "end": 108090338
  }, {
   "filename": "/Images/m65a.c16",
   "start": 108090338,
   "end": 108094494
  }, {
   "filename": "/Images/i42c.c16",
   "start": 108094494,
   "end": 108102778
  }, {
   "filename": "/Images/n40b.c16",
   "start": 108102778,
   "end": 108109516
  }, {
   "filename": "/Images/l20a.c16",
   "start": 108109516,
   "end": 108117084
  }, {
   "filename": "/Images/a04c.c16",
   "start": 108117084,
   "end": 108970594
  }, {
   "filename": "/Images/objects_of_learning.c16",
   "start": 108970594,
   "end": 109525848
  }, {
   "filename": "/Images/m04c.c16",
   "start": 109525848,
   "end": 109533638
  }, {
   "filename": "/Images/b40b.c16",
   "start": 109533638,
   "end": 109620364
  }, {
   "filename": "/Images/chemicalchoice.c16",
   "start": 109620364,
   "end": 109791364
  }, {
   "filename": "/Images/e42c.c16",
   "start": 109791364,
   "end": 109800646
  }, {
   "filename": "/Images/c05b.c16",
   "start": 109800646,
   "end": 109819168
  }, {
   "filename": "/Images/life_events.c16",
   "start": 109819168,
   "end": 109828486
  }, {
   "filename": "/Images/a65a.c16",
   "start": 109828486,
   "end": 110357626
  }, {
   "filename": "/Images/grid.c16",
   "start": 110357626,
   "end": 110394610
  }, {
   "filename": "/Images/whiteontransparentchars.s16",
   "start": 110394610,
   "end": 110426982
  }, {
   "filename": "/Images/m24a.c16",
   "start": 110426982,
   "end": 110430156
  }, {
   "filename": "/Images/trapdoor.c16",
   "start": 110430156,
   "end": 110468008
  }, {
   "filename": "/Images/e62a.c16",
   "start": 110468008,
   "end": 110475870
  }, {
   "filename": "/Images/hummingbird.c16",
   "start": 110475870,
   "end": 110537058
  }, {
   "filename": "/Images/a45c.c16",
   "start": 110537058,
   "end": 111517132
  }, {
   "filename": "/Images/g02b.c16",
   "start": 111517132,
   "end": 111529494
  }, {
   "filename": "/Images/sepulchre.c16",
   "start": 111529494,
   "end": 111653382
  }, {
   "filename": "/Images/m25a.c16",
   "start": 111653382,
   "end": 111656556
  }, {
   "filename": "/Images/hatch.c16",
   "start": 111656556,
   "end": 113540210
  }, {
   "filename": "/Images/i02c.c16",
   "start": 113540210,
   "end": 113548494
  }, {
   "filename": "/Images/n00b.c16",
   "start": 113548494,
   "end": 113554436
  }, {
   "filename": "/Images/l60a.c16",
   "start": 113554436,
   "end": 113562004
  }, {
   "filename": "/Images/a44c.c16",
   "start": 113562004,
   "end": 114514974
  }, {
   "filename": "/Images/handle.c16",
   "start": 114514974,
   "end": 114622984
  }, {
   "filename": "/Images/blank.c16",
   "start": 114622984,
   "end": 114623796
  }, {
   "filename": "/Images/k42b.c16",
   "start": 114623796,
   "end": 114632080
  }, {
   "filename": "/Images/m05c.c16",
   "start": 114632080,
   "end": 114639870
  }, {
   "filename": "/Images/i22a.c16",
   "start": 114639870,
   "end": 114646580
  }, {
   "filename": "/Images/c04b.c16",
   "start": 114646580,
   "end": 114665102
  }, {
   "filename": "/Images/a64a.c16",
   "start": 114665102,
   "end": 115194242
  }, {
   "filename": "/Images/l40c.c16",
   "start": 115194242,
   "end": 115201810
  }, {
   "filename": "/Images/spew.c16",
   "start": 115201810,
   "end": 118388622
  }, {
   "filename": "/Images/m64a.c16",
   "start": 118388622,
   "end": 118391796
  }, {
   "filename": "/Images/learning_machine_buttons.c16",
   "start": 118391796,
   "end": 118409680
  }, {
   "filename": "/Images/e22a.c16",
   "start": 118409680,
   "end": 118417542
  }, {
   "filename": "/Images/a05c.c16",
   "start": 118417542,
   "end": 119274836
  }, {
   "filename": "/Images/g42b.c16",
   "start": 119274836,
   "end": 119287198
  }, {
   "filename": "/Images/m44c.c16",
   "start": 119287198,
   "end": 119294988
  }, {
   "filename": "/Images/b00b.c16",
   "start": 119294988,
   "end": 119374066
  }, {
   "filename": "/Images/e02c.c16",
   "start": 119374066,
   "end": 119383348
  }, {
   "filename": "/Images/c45b.c16",
   "start": 119383348,
   "end": 119401870
  }, {
   "filename": "/Images/a25a.c16",
   "start": 119401870,
   "end": 119921694
  }, {
   "filename": "/Images/g62a.c16",
   "start": 119921694,
   "end": 119931886
  }, {
   "filename": "/Images/c45c.c16",
   "start": 119931886,
   "end": 119950408
  }, {
   "filename": "/Images/e02b.c16",
   "start": 119950408,
   "end": 119959690
  }, {
   "filename": "/Images/b00c.c16",
   "start": 119959690,
   "end": 120038768
  }, {
   "filename": "/Images/m44b.c16",
   "start": 120038768,
   "end": 120046558
  }, {
   "filename": "/Images/wysts.c16",
   "start": 120046558,
   "end": 120078454
  }, {
   "filename": "/Images/g42c.c16",
   "start": 120078454,
   "end": 120090816
  }, {
   "filename": "/Images/a05b.c16",
   "start": 120090816,
   "end": 121022582
  }, {
   "filename": "/Images/airlock_right.c16",
   "start": 121022582,
   "end": 121158744
  }, {
   "filename": "/Images/c65a.c16",
   "start": 121158744,
   "end": 121171968
  }, {
   "filename": "/Images/b20a.c16",
   "start": 121171968,
   "end": 121258342
  }, {
   "filename": "/Images/ocean_pod.c16",
   "start": 121258342,
   "end": 121573980
  }, {
   "filename": "/Images/egglayer.c16",
   "start": 121573980,
   "end": 121651996
  }, {
   "filename": "/Images/recycler.c16",
   "start": 121651996,
   "end": 122235942
  }, {
   "filename": "/Images/eggmakerdisplay.c16",
   "start": 122235942,
   "end": 122808718
  }, {
   "filename": "/Images/l40b.c16",
   "start": 122808718,
   "end": 122816286
  }, {
   "filename": "/Images/n20a.c16",
   "start": 122816286,
   "end": 122818820
  }, {
   "filename": "/Images/c04c.c16",
   "start": 122818820,
   "end": 122837342
  }, {
   "filename": "/Images/000a.c16",
   "start": 122837342,
   "end": 122884004
  }, {
   "filename": "/Images/m05b.c16",
   "start": 122884004,
   "end": 122891794
  }, {
   "filename": "/Images/k42c.c16",
   "start": 122891794,
   "end": 122900078
  }, {
   "filename": "/Images/whiteongreenchars.s16",
   "start": 122900078,
   "end": 122913632
  }, {
   "filename": "/Images/death_cloud.c16",
   "start": 122913632,
   "end": 123259734
  }, {
   "filename": "/Images/a44b.c16",
   "start": 123259734,
   "end": 124143248
  }, {
   "filename": "/Images/c24a.c16",
   "start": 124143248,
   "end": 124156472
  }, {
   "filename": "/Images/n00c.c16",
   "start": 124156472,
   "end": 124162414
  }, {
   "filename": "/Images/i02b.c16",
   "start": 124162414,
   "end": 124170698
  }, {
   "filename": "/Images/k62a.c16",
   "start": 124170698,
   "end": 124177408
  }, {
   "filename": "/Images/norncut01.c16",
   "start": 124177408,
   "end": 124400982
  }, {
   "filename": "/Images/asteroids.c16",
   "start": 124400982,
   "end": 124416292
  }, {
   "filename": "/Images/rock2.c16",
   "start": 124416292,
   "end": 124432906
  }, {
   "filename": "/Images/starplant.c16",
   "start": 124432906,
   "end": 124454190
  }, {
   "filename": "/Images/progress-nl.s16",
   "start": 124454190,
   "end": 124646758
  }, {
   "filename": "/Images/k15a.c16",
   "start": 124646758,
   "end": 124662780
  }, {
   "filename": "/Images/020a.c16",
   "start": 124662780,
   "end": 124709442
  }, {
   "filename": "/Images/n00a.c16",
   "start": 124709442,
   "end": 124715384
  }, {
   "filename": "/Images/j50a.c16",
   "start": 124715384,
   "end": 124729190
  }, {
   "filename": "/Images/welcome screen.c16",
   "start": 124729190,
   "end": 125094134
  }, {
   "filename": "/Images/replicator.c16",
   "start": 125094134,
   "end": 126144282
  }, {
   "filename": "/Images/k42a.c16",
   "start": 126144282,
   "end": 126152566
  }, {
   "filename": "/Images/000c.c16",
   "start": 126152566,
   "end": 126199228
  }, {
   "filename": "/Images/watr.c16",
   "start": 126199228,
   "end": 126200928
  }, {
   "filename": "/Images/c04a.c16",
   "start": 126200928,
   "end": 126219450
  }, {
   "filename": "/Images/g54a.c16",
   "start": 126219450,
   "end": 126236742
  }, {
   "filename": "/Images/sealift.c16",
   "start": 126236742,
   "end": 126246184
  }, {
   "filename": "/Images/angel.c16",
   "start": 126246184,
   "end": 126317698
  }, {
   "filename": "/Images/cactus.c16",
   "start": 126317698,
   "end": 126384758
  }, {
   "filename": "/Images/k54a.c16",
   "start": 126384758,
   "end": 126400780
  }, {
   "filename": "/Images/g42a.c16",
   "start": 126400780,
   "end": 126413142
  }, {
   "filename": "/Images/c12a.c16",
   "start": 126413142,
   "end": 126426366
  }, {
   "filename": "/Images/b00a.c16",
   "start": 126426366,
   "end": 126502852
  }, {
   "filename": "/Images/f50a.c16",
   "start": 126502852,
   "end": 126511602
  }, {
   "filename": "/Images/jaws.c16",
   "start": 126511602,
   "end": 126604136
  }, {
   "filename": "/Images/c45a.c16",
   "start": 126604136,
   "end": 126622658
  }, {
   "filename": "/Images/g15a.c16",
   "start": 126622658,
   "end": 126639950
  }, {
   "filename": "/Images/e02a.c16",
   "start": 126639950,
   "end": 126649232
  }, {
   "filename": "/Images/man-o-war.c16",
   "start": 126649232,
   "end": 127009106
  }, {
   "filename": "/Images/a52a.c16",
   "start": 127009106,
   "end": 127947314
  }, {
   "filename": "/Images/outputdisplay.c16",
   "start": 127947314,
   "end": 127956108
  }, {
   "filename": "/Images/m44a.c16",
   "start": 127956108,
   "end": 127963898
  }, {
   "filename": "/Images/i14a.c16",
   "start": 127963898,
   "end": 127979942
  }, {
   "filename": "/Images/oraponge.c16",
   "start": 127979942,
   "end": 128057858
  }, {
   "filename": "/Images/connectingtest.c16",
   "start": 128057858,
   "end": 128088092
  }, {
   "filename": "/Images/a05a.c16",
   "start": 128088092,
   "end": 128933022
  }, {
   "filename": "/Images/counter.c16",
   "start": 128933022,
   "end": 128988586
  }, {
   "filename": "/Images/e55a.c16",
   "start": 128988586,
   "end": 129009392
  }, {
   "filename": "/Images/d10a.c16",
   "start": 129009392,
   "end": 129017626
  }, {
   "filename": "/Images/deth.c16",
   "start": 129017626,
   "end": 129205946
  }, {
   "filename": "/Images/l40a.c16",
   "start": 129205946,
   "end": 129213514
  }, {
   "filename": "/Images/h10a.c16",
   "start": 129213514,
   "end": 129223672
  }, {
   "filename": "/Images/m05a.c16",
   "start": 129223672,
   "end": 129231462
  }, {
   "filename": "/Images/000b.c16",
   "start": 129231462,
   "end": 129278124
  }, {
   "filename": "/Images/i55a.c16",
   "start": 129278124,
   "end": 129294168
  }, {
   "filename": "/Images/shark.c16",
   "start": 129294168,
   "end": 129364256
  }, {
   "filename": "/Images/airlock_left.c16",
   "start": 129364256,
   "end": 129512362
  }, {
   "filename": "/Images/potions.s16",
   "start": 129512362,
   "end": 129517408
  }, {
   "filename": "/Images/a44a.c16",
   "start": 129517408,
   "end": 130494306
  }, {
   "filename": "/Images/e14a.c16",
   "start": 130494306,
   "end": 130515112
  }, {
   "filename": "/Images/i02a.c16",
   "start": 130515112,
   "end": 130523396
  }, {
   "filename": "/Images/m52a.c16",
   "start": 130523396,
   "end": 130538820
  }, {
   "filename": "/Images/fairy.c16",
   "start": 130538820,
   "end": 130657168
  }, {
   "filename": "/My Agents/Starter Parent 2-de.family",
   "start": 130657168,
   "end": 130719485
  }, {
   "filename": "/My Agents/Starter Parent 2-es.family",
   "start": 130719485,
   "end": 130780301
  }, {
   "filename": "/My Agents/Starter Parent 1-fr.family",
   "start": 130780301,
   "end": 130843252
  }, {
   "filename": "/My Agents/Starter Parent 2.family",
   "start": 130843252,
   "end": 130904726
  }, {
   "filename": "/My Agents/Starter Parent 2-it.family",
   "start": 130904726,
   "end": 130967550
  }, {
   "filename": "/My Agents/Creator-nl.agents",
   "start": 130967550,
   "end": 131020932
  }, {
   "filename": "/My Agents/BruinBengalCivetEggs.agents",
   "start": 131020932,
   "end": 131022122
  }, {
   "filename": "/My Agents/Creator-fr.agents",
   "start": 131022122,
   "end": 131075504
  }, {
   "filename": "/My Agents/Starter Parent 1-nl.family",
   "start": 131075504,
   "end": 131140079
  }, {
   "filename": "/My Agents/Creator-es.agents",
   "start": 131140079,
   "end": 131193461
  }, {
   "filename": "/My Agents/Creator-de.agents",
   "start": 131193461,
   "end": 131246843
  }, {
   "filename": "/My Agents/expressiveeggs.agents",
   "start": 131246843,
   "end": 131356739
  }, {
   "filename": "/My Agents/Creator-it.agents",
   "start": 131356739,
   "end": 131410121
  }, {
   "filename": "/My Agents/Starter Parent 1.family",
   "start": 131410121,
   "end": 131473289
  }, {
   "filename": "/My Agents/Starter Parent 2-nl.family",
   "start": 131473289,
   "end": 131529648
  }, {
   "filename": "/My Agents/Creator-ru.agents",
   "start": 131529648,
   "end": 131583030
  }, {
   "filename": "/My Agents/Starter Parent 2-fr.family",
   "start": 131583030,
   "end": 131644911
  }, {
   "filename": "/My Agents/Starter Parent 1-es.family",
   "start": 131644911,
   "end": 131708113
  }, {
   "filename": "/My Agents/Starter Parent 1-de.family",
   "start": 131708113,
   "end": 131765994
  }, {
   "filename": "/My Agents/Creator.agents",
   "start": 131765994,
   "end": 131819376
  }, {
   "filename": "/My Agents/Starter Parent 1-it.family",
   "start": 131819376,
   "end": 131879890
  }, {
   "filename": "/Sounds/mcls.wav",
   "start": 131879890,
   "end": 132058612,
   "audio": 1
  }, {
   "filename": "/Sounds/rdrl.wav",
   "start": 132058612,
   "end": 132323256,
   "audio": 1
  }, {
   "filename": "/Sounds/pus0.wav",
   "start": 132323256,
   "end": 132324724,
   "audio": 1
  }, {
   "filename": "/Sounds/beh1.wav",
   "start": 132324724,
   "end": 132327448,
   "audio": 1
  }, {
   "filename": "/Sounds/cam1.wav",
   "start": 132327448,
   "end": 132514296,
   "audio": 1
  }, {
   "filename": "/Sounds/alrm.wav",
   "start": 132514296,
   "end": 132602528,
   "audio": 1
  }, {
   "filename": "/Sounds/snor.wav",
   "start": 132602528,
   "end": 133111856,
   "audio": 1
  }, {
   "filename": "/Sounds/yaue.wav",
   "start": 133111856,
   "end": 133115183,
   "audio": 1
  }, {
   "filename": "/Sounds/lg_1.wav",
   "start": 133115183,
   "end": 133185371,
   "audio": 1
  }, {
   "filename": "/Sounds/drm1.wav",
   "start": 133185371,
   "end": 133191232,
   "audio": 1
  }, {
   "filename": "/Sounds/drnk.wav",
   "start": 133191232,
   "end": 133218284,
   "audio": 1
  }, {
   "filename": "/Sounds/ke_m.wav",
   "start": 133218284,
   "end": 133221472,
   "audio": 1
  }, {
   "filename": "/Sounds/bohm.wav",
   "start": 133221472,
   "end": 133223588,
   "audio": 1
  }, {
   "filename": "/Sounds/step.wav",
   "start": 133223588,
   "end": 133226576,
   "audio": 1
  }, {
   "filename": "/Sounds/foom.wav",
   "start": 133226576,
   "end": 133229191,
   "audio": 1
  }, {
   "filename": "/Sounds/st_1.wav",
   "start": 133229191,
   "end": 133446621,
   "audio": 1
  }, {
   "filename": "/Sounds/zyoe.wav",
   "start": 133446621,
   "end": 133450650,
   "audio": 1
  }, {
   "filename": "/Sounds/che2.wav",
   "start": 133450650,
   "end": 133511941,
   "audio": 1
  }, {
   "filename": "/Sounds/beh0.wav",
   "start": 133511941,
   "end": 133514715,
   "audio": 1
  }, {
   "filename": "/Sounds/nohm.wav",
   "start": 133514715,
   "end": 133518735,
   "audio": 1
  }, {
   "filename": "/Sounds/pus1.wav",
   "start": 133518735,
   "end": 133520183,
   "audio": 1
  }, {
   "filename": "/Sounds/seim.wav",
   "start": 133520183,
   "end": 133523359,
   "audio": 1
  }, {
   "filename": "/Sounds/cmc4.wav",
   "start": 133523359,
   "end": 133752963,
   "audio": 1
  }, {
   "filename": "/Sounds/gsnr.wav",
   "start": 133752963,
   "end": 133793263,
   "audio": 1
  }, {
   "filename": "/Sounds/cam2.wav",
   "start": 133793263,
   "end": 133824245,
   "audio": 1
  }, {
   "filename": "/Sounds/st_3.wav",
   "start": 133824245,
   "end": 133924187,
   "audio": 1
  }, {
   "filename": "/Sounds/zyo1.wav",
   "start": 133924187,
   "end": 133928161,
   "audio": 1
  }, {
   "filename": "/Sounds/yau0.wav",
   "start": 133928161,
   "end": 133931489,
   "audio": 1
  }, {
   "filename": "/Sounds/pop_.wav",
   "start": 133931489,
   "end": 133937823,
   "audio": 1
  }, {
   "filename": "/Sounds/mwrn.wav",
   "start": 133937823,
   "end": 133943150,
   "audio": 1
  }, {
   "filename": "/Sounds/drm2.wav",
   "start": 133943150,
   "end": 133950256,
   "audio": 1
  }, {
   "filename": "/Sounds/kahm.wav",
   "start": 133950256,
   "end": 133953164,
   "audio": 1
  }, {
   "filename": "/Sounds/nlnl.wav",
   "start": 133953164,
   "end": 133963628,
   "audio": 1
  }, {
   "filename": "/Sounds/cs_1.wav",
   "start": 133963628,
   "end": 134066342,
   "audio": 1
  }, {
   "filename": "/Sounds/yau1.wav",
   "start": 134066342,
   "end": 134069720,
   "audio": 1
  }, {
   "filename": "/Sounds/st_2.wav",
   "start": 134069720,
   "end": 134278748,
   "audio": 1
  }, {
   "filename": "/Sounds/gsb1.wav",
   "start": 134278748,
   "end": 134558854,
   "audio": 1
  }, {
   "filename": "/Sounds/zyo0.wav",
   "start": 134558854,
   "end": 134562884,
   "audio": 1
  }, {
   "filename": "/Sounds/spew.wav",
   "start": 134562884,
   "end": 134691446,
   "audio": 1
  }, {
   "filename": "/Sounds/behe.wav",
   "start": 134691446,
   "end": 134694220,
   "audio": 1
  }, {
   "filename": "/Sounds/che1.wav",
   "start": 134694220,
   "end": 134755511,
   "audio": 1
  }, {
   "filename": "/Sounds/move.wav",
   "start": 134755511,
   "end": 134779237,
   "audio": 1
  }, {
   "filename": "/Sounds/cmc5.wav",
   "start": 134779237,
   "end": 135796653,
   "audio": 1
  }, {
   "filename": "/Sounds/cmc1.wav",
   "start": 135796653,
   "end": 135931617,
   "audio": 1
  }, {
   "filename": "/Sounds/gr_1.wav",
   "start": 135931617,
   "end": 135982611,
   "audio": 1
  }, {
   "filename": "/Sounds/gr_p.wav",
   "start": 135982611,
   "end": 136009957,
   "audio": 1
  }, {
   "filename": "/Sounds/sgfr.wav",
   "start": 136009957,
   "end": 136074079,
   "audio": 1
  }, {
   "filename": "/Sounds/bee0.wav",
   "start": 136074079,
   "end": 136078229,
   "audio": 1
  }, {
   "filename": "/Sounds/koh0.wav",
   "start": 136078229,
   "end": 136080429,
   "audio": 1
  }, {
   "filename": "/Sounds/crak.wav",
   "start": 136080429,
   "end": 136163901,
   "audio": 1
  }, {
   "filename": "/Sounds/yoou.wav",
   "start": 136163901,
   "end": 136177381,
   "audio": 1
  }, {
   "filename": "/Sounds/koh1.wav",
   "start": 136177381,
   "end": 136179695,
   "audio": 1
  }, {
   "filename": "/Sounds/esb1.wav",
   "start": 136179695,
   "end": 136329851,
   "audio": 1
  }, {
   "filename": "/Sounds/bee1.wav",
   "start": 136329851,
   "end": 136334033,
   "audio": 1
  }, {
   "filename": "/Sounds/env2.wav",
   "start": 136334033,
   "end": 136598763,
   "audio": 1
  }, {
   "filename": "/Sounds/beep.wav",
   "start": 136598763,
   "end": 136617143,
   "audio": 1
  }, {
   "filename": "/Sounds/sirn.wav",
   "start": 136617143,
   "end": 136624584,
   "audio": 1
  }, {
   "filename": "/Sounds/ccls.wav",
   "start": 136624584,
   "end": 136626868,
   "audio": 1
  }, {
   "filename": "/Sounds/bep2.wav",
   "start": 136626868,
   "end": 136645248,
   "audio": 1
  }, {
   "filename": "/Sounds/cmc2.wav",
   "start": 136645248,
   "end": 137025620,
   "audio": 1
  }, {
   "filename": "/Sounds/but1.wav",
   "start": 137025620,
   "end": 137164596,
   "audio": 1
  }, {
   "filename": "/Sounds/gr_2.wav",
   "start": 137164596,
   "end": 137247596,
   "audio": 1
  }, {
   "filename": "/Sounds/pkup.wav",
   "start": 137247596,
   "end": 137459724,
   "audio": 1
  }, {
   "filename": "/Sounds/ghop.wav",
   "start": 137459724,
   "end": 137521850,
   "audio": 1
  }, {
   "filename": "/Sounds/beee.wav",
   "start": 137521850,
   "end": 137525999,
   "audio": 1
  }, {
   "filename": "/Sounds/grendel.mng",
   "start": 137525999,
   "end": 142592508
  }, {
   "filename": "/Sounds/bost.wav",
   "start": 142592508,
   "end": 142622531,
   "audio": 1
  }, {
   "filename": "/Sounds/tckl.wav",
   "start": 142622531,
   "end": 142633513,
   "audio": 1
  }, {
   "filename": "/Sounds/kohe.wav",
   "start": 142633513,
   "end": 142635813,
   "audio": 1
  }, {
   "filename": "/Sounds/ee_m.wav",
   "start": 142635813,
   "end": 142637415,
   "audio": 1
  }, {
   "filename": "/Sounds/engineering.mng",
   "start": 142637415,
   "end": 145301894
  }, {
   "filename": "/Sounds/env1.wav",
   "start": 145301894,
   "end": 145499466,
   "audio": 1
  }, {
   "filename": "/Sounds/garr.wav",
   "start": 145499466,
   "end": 145507206,
   "audio": 1
  }, {
   "filename": "/Sounds/know.wav",
   "start": 145507206,
   "end": 145730296,
   "audio": 1
  }, {
   "filename": "/Sounds/gr_e.wav",
   "start": 145730296,
   "end": 145889614,
   "audio": 1
  }, {
   "filename": "/Sounds/egg1.wav",
   "start": 145889614,
   "end": 145968902,
   "audio": 1
  }, {
   "filename": "/Sounds/buzz.wav",
   "start": 145968902,
   "end": 145989624,
   "audio": 1
  }, {
   "filename": "/Sounds/cmc3.wav",
   "start": 145989624,
   "end": 146009072,
   "audio": 1
  }, {
   "filename": "/Sounds/berm.wav",
   "start": 146009072,
   "end": 146011212,
   "audio": 1
  }, {
   "filename": "/Sounds/rahe.wav",
   "start": 146011212,
   "end": 146014150,
   "audio": 1
  }, {
   "filename": "/Sounds/ha_0.wav",
   "start": 146014150,
   "end": 146016816,
   "audio": 1
  }, {
   "filename": "/Sounds/wehm.wav",
   "start": 146016816,
   "end": 146019707,
   "audio": 1
  }, {
   "filename": "/Sounds/mahm.wav",
   "start": 146019707,
   "end": 146023015,
   "audio": 1
  }, {
   "filename": "/Sounds/snee.wav",
   "start": 146023015,
   "end": 146029545,
   "audio": 1
  }, {
   "filename": "/Sounds/dr_o.wav",
   "start": 146029545,
   "end": 146090103,
   "audio": 1
  }, {
   "filename": "/Sounds/sl_1.wav",
   "start": 146090103,
   "end": 146409413,
   "audio": 1
  }, {
   "filename": "/Sounds/eat1.wav",
   "start": 146409413,
   "end": 146445577,
   "audio": 1
  }, {
   "filename": "/Sounds/oogh.wav",
   "start": 146445577,
   "end": 146452359,
   "audio": 1
  }, {
   "filename": "/Sounds/gui1.wav",
   "start": 146452359,
   "end": 146541287,
   "audio": 1
  }, {
   "filename": "/Sounds/soo0.wav",
   "start": 146541287,
   "end": 146544803,
   "audio": 1
  }, {
   "filename": "/Sounds/mno0.wav",
   "start": 146544803,
   "end": 146549327,
   "audio": 1
  }, {
   "filename": "/Sounds/excl.wav",
   "start": 146549327,
   "end": 146579483,
   "audio": 1
  }, {
   "filename": "/Sounds/woh0.wav",
   "start": 146579483,
   "end": 146582263,
   "audio": 1
  }, {
   "filename": "/Sounds/gs_c.wav",
   "start": 146582263,
   "end": 146678445,
   "audio": 1
  }, {
   "filename": "/Sounds/woh1.wav",
   "start": 146678445,
   "end": 146681225,
   "audio": 1
  }, {
   "filename": "/Sounds/grun.wav",
   "start": 146681225,
   "end": 146687383,
   "audio": 1
  }, {
   "filename": "/Sounds/clak.wav",
   "start": 146687383,
   "end": 146694079,
   "audio": 1
  }, {
   "filename": "/Sounds/soo1.wav",
   "start": 146694079,
   "end": 146697149,
   "audio": 1
  }, {
   "filename": "/Sounds/shiv.wav",
   "start": 146697149,
   "end": 146731237,
   "audio": 1
  }, {
   "filename": "/Sounds/mno1.wav",
   "start": 146731237,
   "end": 146736343,
   "audio": 1
  }, {
   "filename": "/Sounds/eshv.wav",
   "start": 146736343,
   "end": 146770430,
   "audio": 1
  }, {
   "filename": "/Sounds/goww.wav",
   "start": 146770430,
   "end": 146780778,
   "audio": 1
  }, {
   "filename": "/Sounds/rad1.wav",
   "start": 146780778,
   "end": 146794972,
   "audio": 1
  }, {
   "filename": "/Sounds/rok1.wav",
   "start": 146794972,
   "end": 146815652,
   "audio": 1
  }, {
   "filename": "/Sounds/ha_1.wav",
   "start": 146815652,
   "end": 146818284,
   "audio": 1
  }, {
   "filename": "/Sounds/flap.wav",
   "start": 146818284,
   "end": 146898938,
   "audio": 1
  }, {
   "filename": "/Sounds/rah0.wav",
   "start": 146898938,
   "end": 146901878,
   "audio": 1
  }, {
   "filename": "/Sounds/radr.wav",
   "start": 146901878,
   "end": 146905456,
   "audio": 1
  }, {
   "filename": "/Sounds/argh.wav",
   "start": 146905456,
   "end": 146914562,
   "audio": 1
  }, {
   "filename": "/Sounds/ha_e.wav",
   "start": 146914562,
   "end": 146917666,
   "audio": 1
  }, {
   "filename": "/Sounds/pir2.wav",
   "start": 146917666,
   "end": 147759458,
   "audio": 1
  }, {
   "filename": "/Sounds/sqh3.wav",
   "start": 147759458,
   "end": 147882574,
   "audio": 1
  }, {
   "filename": "/Sounds/mnoe.wav",
   "start": 147882574,
   "end": 147887631,
   "audio": 1
  }, {
   "filename": "/Sounds/oarr.wav",
   "start": 147887631,
   "end": 147896939,
   "audio": 1
  }, {
   "filename": "/Sounds/pohm.wav",
   "start": 147896939,
   "end": 147899948,
   "audio": 1
  }, {
   "filename": "/Sounds/ct_4.wav",
   "start": 147899948,
   "end": 148167308,
   "audio": 1
  }, {
   "filename": "/Sounds/btle.wav",
   "start": 148167308,
   "end": 148177700,
   "audio": 1
  }, {
   "filename": "/Sounds/gui3.wav",
   "start": 148177700,
   "end": 148268272,
   "audio": 1
  }, {
   "filename": "/Sounds/rasm.wav",
   "start": 148268272,
   "end": 148270708,
   "audio": 1
  }, {
   "filename": "/Sounds/ragh.wav",
   "start": 148270708,
   "end": 148278004,
   "audio": 1
  }, {
   "filename": "/Sounds/rok2.wav",
   "start": 148278004,
   "end": 148422772,
   "audio": 1
  }, {
   "filename": "/Sounds/rah1.wav",
   "start": 148422772,
   "end": 148425726,
   "audio": 1
  }, {
   "filename": "/Sounds/burp.wav",
   "start": 148425726,
   "end": 148436049,
   "audio": 1
  }, {
   "filename": "/Sounds/gauw.wav",
   "start": 148436049,
   "end": 148444773,
   "audio": 1
  }, {
   "filename": "/Sounds/dah1.wav",
   "start": 148444773,
   "end": 148447233,
   "audio": 1
  }, {
   "filename": "/Sounds/pnt1.wav",
   "start": 148447233,
   "end": 148512499,
   "audio": 1
  }, {
   "filename": "/Sounds/beam.wav",
   "start": 148512499,
   "end": 148533323,
   "audio": 1
  }, {
   "filename": "/Sounds/chn1.wav",
   "start": 148533323,
   "end": 148536023,
   "audio": 1
  }, {
   "filename": "/Sounds/niy0.wav",
   "start": 148536023,
   "end": 148538401,
   "audio": 1
  }, {
   "filename": "/Sounds/pah0.wav",
   "start": 148538401,
   "end": 148541277,
   "audio": 1
  }, {
   "filename": "/Sounds/dr10.wav",
   "start": 148541277,
   "end": 148546781,
   "audio": 1
  }, {
   "filename": "/Sounds/asba.wav",
   "start": 148546781,
   "end": 148554289,
   "audio": 1
  }, {
   "filename": "/Sounds/crea.wav",
   "start": 148554289,
   "end": 148774767,
   "audio": 1
  }, {
   "filename": "/Sounds/pah1.wav",
   "start": 148774767,
   "end": 148777607,
   "audio": 1
  }, {
   "filename": "/Sounds/ct_1.wav",
   "start": 148777607,
   "end": 148892599,
   "audio": 1
  }, {
   "filename": "/Sounds/niy1.wav",
   "start": 148892599,
   "end": 148894971,
   "audio": 1
  }, {
   "filename": "/Sounds/chn0.wav",
   "start": 148894971,
   "end": 148897783,
   "audio": 1
  }, {
   "filename": "/Sounds/weem.wav",
   "start": 148897783,
   "end": 148900179,
   "audio": 1
  }, {
   "filename": "/Sounds/gate.wav",
   "start": 148900179,
   "end": 148929345,
   "audio": 1
  }, {
   "filename": "/Sounds/dah0.wav",
   "start": 148929345,
   "end": 148931569,
   "audio": 1
  }, {
   "filename": "/Sounds/rckl.wav",
   "start": 148931569,
   "end": 148944655,
   "audio": 1
  }, {
   "filename": "/Sounds/dor1.wav",
   "start": 148944655,
   "end": 149381787,
   "audio": 1
  }, {
   "filename": "/Sounds/ow!1.wav",
   "start": 149381787,
   "end": 149385363,
   "audio": 1
  }, {
   "filename": "/Sounds/dslg.wav",
   "start": 149385363,
   "end": 149737569,
   "audio": 1
  }, {
   "filename": "/Sounds/ow!3.wav",
   "start": 149737569,
   "end": 149743677,
   "audio": 1
  }, {
   "filename": "/Sounds/epng.wav",
   "start": 149743677,
   "end": 149831031,
   "audio": 1
  }, {
   "filename": "/Sounds/trap.wav",
   "start": 149831031,
   "end": 149840035,
   "audio": 1
  }, {
   "filename": "/Sounds/chw0.wav",
   "start": 149840035,
   "end": 149844095,
   "audio": 1
  }, {
   "filename": "/Sounds/cahm.wav",
   "start": 149844095,
   "end": 149846690,
   "audio": 1
  }, {
   "filename": "/Sounds/niye.wav",
   "start": 149846690,
   "end": 149849067,
   "audio": 1
  }, {
   "filename": "/Sounds/nudi.wav",
   "start": 149849067,
   "end": 149877997,
   "audio": 1
  }, {
   "filename": "/Sounds/ct_3.wav",
   "start": 149877997,
   "end": 149948043,
   "audio": 1
  }, {
   "filename": "/Sounds/pahe.wav",
   "start": 149948043,
   "end": 149950970,
   "audio": 1
  }, {
   "filename": "/Sounds/wahm.wav",
   "start": 149950970,
   "end": 149954046,
   "audio": 1
  }, {
   "filename": "/Sounds/yhan.wav",
   "start": 149954046,
   "end": 149964138,
   "audio": 1
  }, {
   "filename": "/Sounds/do_m.wav",
   "start": 149964138,
   "end": 149966682,
   "audio": 1
  }, {
   "filename": "/Sounds/ct_2.wav",
   "start": 149966682,
   "end": 150013628,
   "audio": 1
  }, {
   "filename": "/Sounds/wasp.wav",
   "start": 150013628,
   "end": 150187928,
   "audio": 1
  }, {
   "filename": "/Sounds/gror.wav",
   "start": 150187928,
   "end": 150194994,
   "audio": 1
  }, {
   "filename": "/Sounds/chne.wav",
   "start": 150194994,
   "end": 150197908,
   "audio": 1
  }, {
   "filename": "/Sounds/ndid.wav",
   "start": 150197908,
   "end": 150257390,
   "audio": 1
  }, {
   "filename": "/Sounds/dahe.wav",
   "start": 150257390,
   "end": 150260033,
   "audio": 1
  }, {
   "filename": "/Sounds/gshv.wav",
   "start": 150260033,
   "end": 150270941,
   "audio": 1
  }, {
   "filename": "/Sounds/chw1.wav",
   "start": 150270941,
   "end": 150274847,
   "audio": 1
  }, {
   "filename": "/Sounds/chwp.wav",
   "start": 150274847,
   "end": 150284283,
   "audio": 1
  }, {
   "filename": "/Sounds/dor2.wav",
   "start": 150284283,
   "end": 150718421,
   "audio": 1
  }, {
   "filename": "/Sounds/ow!2.wav",
   "start": 150718421,
   "end": 150725265,
   "audio": 1
  }, {
   "filename": "/Sounds/map4.wav",
   "start": 150725265,
   "end": 150912383,
   "audio": 1
  }, {
   "filename": "/Sounds/gdie.wav",
   "start": 150912383,
   "end": 150943851,
   "audio": 1
  }, {
   "filename": "/Sounds/ras0.wav",
   "start": 150943851,
   "end": 150946287,
   "audio": 1
  }, {
   "filename": "/Sounds/me_1.wav",
   "start": 150946287,
   "end": 150950637,
   "audio": 1
  }, {
   "filename": "/Sounds/doh1.wav",
   "start": 150950637,
   "end": 150952617,
   "audio": 1
  }, {
   "filename": "/Sounds/zzzz.wav",
   "start": 150952617,
   "end": 151070677,
   "audio": 1
  }, {
   "filename": "/Sounds/ecof.wav",
   "start": 151070677,
   "end": 151076352,
   "audio": 1
  }, {
   "filename": "/Sounds/warn.wav",
   "start": 151076352,
   "end": 151651796,
   "audio": 1
  }, {
   "filename": "/Sounds/cyc2.wav",
   "start": 151651796,
   "end": 151839276,
   "audio": 1
  }, {
   "filename": "/Sounds/pi_1.wav",
   "start": 151839276,
   "end": 151919494,
   "audio": 1
  }, {
   "filename": "/Sounds/grl2.wav",
   "start": 151919494,
   "end": 152031136,
   "audio": 1
  }, {
   "filename": "/Sounds/stm1.wav",
   "start": 152031136,
   "end": 152272538,
   "audio": 1
  }, {
   "filename": "/Sounds/toww.wav",
   "start": 152272538,
   "end": 152489114,
   "audio": 1
  }, {
   "filename": "/Sounds/poh0.wav",
   "start": 152489114,
   "end": 152492124,
   "audio": 1
  }, {
   "filename": "/Sounds/poh1.wav",
   "start": 152492124,
   "end": 152495026,
   "audio": 1
  }, {
   "filename": "/Sounds/guio.wav",
   "start": 152495026,
   "end": 152627242,
   "audio": 1
  }, {
   "filename": "/Sounds/doh0.wav",
   "start": 152627242,
   "end": 152629356,
   "audio": 1
  }, {
   "filename": "/Sounds/me_0.wav",
   "start": 152629356,
   "end": 152633998,
   "audio": 1
  }, {
   "filename": "/Sounds/wehe.wav",
   "start": 152633998,
   "end": 152636889,
   "audio": 1
  }, {
   "filename": "/Sounds/mahe.wav",
   "start": 152636889,
   "end": 152640197,
   "audio": 1
  }, {
   "filename": "/Sounds/ras1.wav",
   "start": 152640197,
   "end": 152642709,
   "audio": 1
  }, {
   "filename": "/Sounds/rahm.wav",
   "start": 152642709,
   "end": 152645648,
   "audio": 1
  }, {
   "filename": "/Sounds/alar.wav",
   "start": 152645648,
   "end": 152795104,
   "audio": 1
  }, {
   "filename": "/Sounds/events.mng",
   "start": 152795104,
   "end": 155075212
  }, {
   "filename": "/Sounds/mopn.wav",
   "start": 155075212,
   "end": 155253934,
   "audio": 1
  }, {
   "filename": "/Sounds/weh1.wav",
   "start": 155253934,
   "end": 155256714,
   "audio": 1
  }, {
   "filename": "/Sounds/mah1.wav",
   "start": 155256714,
   "end": 155259968,
   "audio": 1
  }, {
   "filename": "/Sounds/rase.wav",
   "start": 155259968,
   "end": 155263734,
   "audio": 1
  }, {
   "filename": "/Sounds/open.wav",
   "start": 155263734,
   "end": 155462864,
   "audio": 1
  }, {
   "filename": "/Sounds/ug_1.wav",
   "start": 155462864,
   "end": 155515504,
   "audio": 1
  }, {
   "filename": "/Sounds/cyc1.wav",
   "start": 155515504,
   "end": 155766120,
   "audio": 1
  }, {
   "filename": "/Sounds/grl1.wav",
   "start": 155766120,
   "end": 155870046,
   "audio": 1
  }, {
   "filename": "/Sounds/pohe.wav",
   "start": 155870046,
   "end": 155873055,
   "audio": 1
  }, {
   "filename": "/Sounds/inje.wav",
   "start": 155873055,
   "end": 155945601,
   "audio": 1
  }, {
   "filename": "/Sounds/pod1.wav",
   "start": 155945601,
   "end": 156142763,
   "audio": 1
  }, {
   "filename": "/Sounds/mnom.wav",
   "start": 156142763,
   "end": 156147287,
   "audio": 1
  }, {
   "filename": "/Sounds/cbz2.wav",
   "start": 156147287,
   "end": 156278743,
   "audio": 1
  }, {
   "filename": "/Sounds/grnn.wav",
   "start": 156278743,
   "end": 156289923,
   "audio": 1
  }, {
   "filename": "/Sounds/mah0.wav",
   "start": 156289923,
   "end": 156293231,
   "audio": 1
  }, {
   "filename": "/Sounds/weh0.wav",
   "start": 156293231,
   "end": 156296123,
   "audio": 1
  }, {
   "filename": "/Sounds/ha_m.wav",
   "start": 156296123,
   "end": 156298788,
   "audio": 1
  }, {
   "filename": "/Sounds/sgtn.wav",
   "start": 156298788,
   "end": 156330368,
   "audio": 1
  }, {
   "filename": "/Sounds/spup.wav",
   "start": 156330368,
   "end": 156395198,
   "audio": 1
  }, {
   "filename": "/Sounds/rain.wav",
   "start": 156395198,
   "end": 157036960,
   "audio": 1
  }, {
   "filename": "/Sounds/map2.wav",
   "start": 157036960,
   "end": 157054604,
   "audio": 1
  }, {
   "filename": "/Sounds/fish.wav",
   "start": 157054604,
   "end": 157175240,
   "audio": 1
  }, {
   "filename": "/Sounds/copn.wav",
   "start": 157175240,
   "end": 157177524,
   "audio": 1
  }, {
   "filename": "/Sounds/hit_.wav",
   "start": 157177524,
   "end": 157256994,
   "audio": 1
  }, {
   "filename": "/Sounds/cah1.wav",
   "start": 157256994,
   "end": 157259992,
   "audio": 1
  }, {
   "filename": "/Sounds/weee.wav",
   "start": 157259992,
   "end": 157262388,
   "audio": 1
  }, {
   "filename": "/Sounds/coil.wav",
   "start": 157262388,
   "end": 157352234,
   "audio": 1
  }, {
   "filename": "/Sounds/stup.wav",
   "start": 157352234,
   "end": 157394430,
   "audio": 1
  }, {
   "filename": "/Sounds/do_0.wav",
   "start": 157394430,
   "end": 157396974,
   "audio": 1
  }, {
   "filename": "/Sounds/wah0.wav",
   "start": 157396974,
   "end": 157400050,
   "audio": 1
  }, {
   "filename": "/Sounds/fp_1.wav",
   "start": 157400050,
   "end": 157451010,
   "audio": 1
  }, {
   "filename": "/Sounds/quen.wav",
   "start": 157451010,
   "end": 157593670,
   "audio": 1
  }, {
   "filename": "/Sounds/wah1.wav",
   "start": 157593670,
   "end": 157596918,
   "audio": 1
  }, {
   "filename": "/Sounds/do_1.wav",
   "start": 157596918,
   "end": 157599794,
   "audio": 1
  }, {
   "filename": "/Sounds/pop1.wav",
   "start": 157599794,
   "end": 157602078,
   "audio": 1
  }, {
   "filename": "/Sounds/music.mng",
   "start": 157602078,
   "end": 165949872
  }, {
   "filename": "/Sounds/text.wav",
   "start": 165949872,
   "end": 165956568,
   "audio": 1
  }, {
   "filename": "/Sounds/cah0.wav",
   "start": 165956568,
   "end": 165959164,
   "audio": 1
  }, {
   "filename": "/Sounds/brng.wav",
   "start": 165959164,
   "end": 165973188,
   "audio": 1
  }, {
   "filename": "/Sounds/map3.wav",
   "start": 165973188,
   "end": 166104140,
   "audio": 1
  }, {
   "filename": "/Sounds/map1.wav",
   "start": 166104140,
   "end": 166238680,
   "audio": 1
  }, {
   "filename": "/Sounds/roh0.wav",
   "start": 166238680,
   "end": 166242968,
   "audio": 1
  }, {
   "filename": "/Sounds/dahm.wav",
   "start": 166242968,
   "end": 166245191,
   "audio": 1
  }, {
   "filename": "/Sounds/punc.wav",
   "start": 166245191,
   "end": 166251243,
   "audio": 1
  }, {
   "filename": "/Sounds/wee0.wav",
   "start": 166251243,
   "end": 166253639,
   "audio": 1
  }, {
   "filename": "/Sounds/dr_c.wav",
   "start": 166253639,
   "end": 166320775,
   "audio": 1
  }, {
   "filename": "/Sounds/chnm.wav",
   "start": 166320775,
   "end": 166323586,
   "audio": 1
  }, {
   "filename": "/Sounds/edie.wav",
   "start": 166323586,
   "end": 166339210,
   "audio": 1
  }, {
   "filename": "/Sounds/foh1.wav",
   "start": 166339210,
   "end": 166341698,
   "audio": 1
  }, {
   "filename": "/Sounds/bridge.mng",
   "start": 166341698,
   "end": 168109544
  }, {
   "filename": "/Sounds/hart.wav",
   "start": 168109544,
   "end": 168122911,
   "audio": 1
  }, {
   "filename": "/Sounds/wahe.wav",
   "start": 168122911,
   "end": 168126227,
   "audio": 1
  }, {
   "filename": "/Sounds/do_e.wav",
   "start": 168126227,
   "end": 168129275,
   "audio": 1
  }, {
   "filename": "/Sounds/fp_2.wav",
   "start": 168129275,
   "end": 168336893,
   "audio": 1
  }, {
   "filename": "/Sounds/gs_o.wav",
   "start": 168336893,
   "end": 168433075,
   "audio": 1
  }, {
   "filename": "/Sounds/bzup.wav",
   "start": 168433075,
   "end": 168594205,
   "audio": 1
  }, {
   "filename": "/Sounds/aqua.wav",
   "start": 168594205,
   "end": 168620109,
   "audio": 1
  }, {
   "filename": "/Sounds/driv.wav",
   "start": 168620109,
   "end": 168732961,
   "audio": 1
  }, {
   "filename": "/Sounds/pahm.wav",
   "start": 168732961,
   "end": 168735836,
   "audio": 1
  }, {
   "filename": "/Sounds/foh0.wav",
   "start": 168735836,
   "end": 168738234,
   "audio": 1
  }, {
   "filename": "/Sounds/cbuz.wav",
   "start": 168738234,
   "end": 168861212,
   "audio": 1
  }, {
   "filename": "/Sounds/niym.wav",
   "start": 168861212,
   "end": 168863589,
   "audio": 1
  }, {
   "filename": "/Sounds/wee1.wav",
   "start": 168863589,
   "end": 168865965,
   "audio": 1
  }, {
   "filename": "/Sounds/spnk.wav",
   "start": 168865965,
   "end": 168869469,
   "audio": 1
  }, {
   "filename": "/Sounds/splt.wav",
   "start": 168869469,
   "end": 168897581,
   "audio": 1
  }, {
   "filename": "/Sounds/cahe.wav",
   "start": 168897581,
   "end": 168900176,
   "audio": 1
  }, {
   "filename": "/Sounds/roh1.wav",
   "start": 168900176,
   "end": 168904416,
   "audio": 1
  }, {
   "filename": "/Sounds/tele.wav",
   "start": 168904416,
   "end": 168950874,
   "audio": 1
  }, {
   "filename": "/Sounds/sei1.wav",
   "start": 168950874,
   "end": 168952882,
   "audio": 1
  }, {
   "filename": "/Sounds/cav2.wav",
   "start": 168952882,
   "end": 169163072,
   "audio": 1
  }, {
   "filename": "/Sounds/nohe.wav",
   "start": 169163072,
   "end": 169167092,
   "audio": 1
  }, {
   "filename": "/Sounds/coug.wav",
   "start": 169167092,
   "end": 169172768,
   "audio": 1
  }, {
   "filename": "/Sounds/snap.wav",
   "start": 169172768,
   "end": 169233484,
   "audio": 1
  }, {
   "filename": "/Sounds/gslp.wav",
   "start": 169233484,
   "end": 169242328,
   "audio": 1
  }, {
   "filename": "/Sounds/main.mng",
   "start": 169242328,
   "end": 177149105
  }, {
   "filename": "/Sounds/mb_4.wav",
   "start": 177149105,
   "end": 177760275,
   "audio": 1
  }, {
   "filename": "/Sounds/zyom.wav",
   "start": 177760275,
   "end": 177764304,
   "audio": 1
  }, {
   "filename": "/Sounds/bubf.wav",
   "start": 177764304,
   "end": 177819900,
   "audio": 1
  }, {
   "filename": "/Sounds/wav3.wav",
   "start": 177819900,
   "end": 177938642,
   "audio": 1
  }, {
   "filename": "/Sounds/dj_g.wav",
   "start": 177938642,
   "end": 180078128,
   "audio": 1
  }, {
   "filename": "/Sounds/fooe.wav",
   "start": 180078128,
   "end": 180081431,
   "audio": 1
  }, {
   "filename": "/Sounds/exp1.wav",
   "start": 180081431,
   "end": 180364097,
   "audio": 1
  }, {
   "filename": "/Sounds/kah0.wav",
   "start": 180364097,
   "end": 180367005,
   "audio": 1
  }, {
   "filename": "/Sounds/hat3.wav",
   "start": 180367005,
   "end": 180455257,
   "audio": 1
  }, {
   "filename": "/Sounds/atms.wav",
   "start": 180455257,
   "end": 180526027,
   "audio": 1
  }, {
   "filename": "/Sounds/ke_e.wav",
   "start": 180526027,
   "end": 180529215,
   "audio": 1
  }, {
   "filename": "/Sounds/clik.wav",
   "start": 180529215,
   "end": 180531603,
   "audio": 1
  }, {
   "filename": "/Sounds/bohe.wav",
   "start": 180531603,
   "end": 180533732,
   "audio": 1
  }, {
   "filename": "/Sounds/scd1.wav",
   "start": 180533732,
   "end": 180659176,
   "audio": 1
  }, {
   "filename": "/Sounds/hat2.wav",
   "start": 180659176,
   "end": 180944056,
   "audio": 1
  }, {
   "filename": "/Sounds/kah1.wav",
   "start": 180944056,
   "end": 180946924,
   "audio": 1
  }, {
   "filename": "/Sounds/gna1.wav",
   "start": 180946924,
   "end": 181010052,
   "audio": 1
  }, {
   "filename": "/Sounds/bp_1.wav",
   "start": 181010052,
   "end": 181060620,
   "audio": 1
  }, {
   "filename": "/Sounds/lg_o.wav",
   "start": 181060620,
   "end": 181084724,
   "audio": 1
  }, {
   "filename": "/Sounds/yaum.wav",
   "start": 181084724,
   "end": 181088051,
   "audio": 1
  }, {
   "filename": "/Sounds/wav2.wav",
   "start": 181088051,
   "end": 181186783,
   "audio": 1
  }, {
   "filename": "/Sounds/kis2.wav",
   "start": 181186783,
   "end": 181254027,
   "audio": 1
  }, {
   "filename": "/Sounds/spdn.wav",
   "start": 181254027,
   "end": 181319183,
   "audio": 1
  }, {
   "filename": "/Sounds/col2.wav",
   "start": 181319183,
   "end": 181409029,
   "audio": 1
  }, {
   "filename": "/Sounds/cav3.wav",
   "start": 181409029,
   "end": 181606191,
   "audio": 1
  }, {
   "filename": "/Sounds/sei0.wav",
   "start": 181606191,
   "end": 181609367,
   "audio": 1
  }, {
   "filename": "/Sounds/cav1.wav",
   "start": 181609367,
   "end": 181750209,
   "audio": 1
  }, {
   "filename": "/Sounds/noh0.wav",
   "start": 181750209,
   "end": 181754229,
   "audio": 1
  }, {
   "filename": "/Sounds/behm.wav",
   "start": 181754229,
   "end": 181757003,
   "audio": 1
  }, {
   "filename": "/Sounds/fly_.wav",
   "start": 181757003,
   "end": 182008743,
   "audio": 1
  }, {
   "filename": "/Sounds/brd5.wav",
   "start": 182008743,
   "end": 182084729,
   "audio": 1
  }, {
   "filename": "/Sounds/weak.wav",
   "start": 182084729,
   "end": 182104658,
   "audio": 1
  }, {
   "filename": "/Sounds/exp2.wav",
   "start": 182104658,
   "end": 182285766,
   "audio": 1
  }, {
   "filename": "/Sounds/foo0.wav",
   "start": 182285766,
   "end": 182288382,
   "audio": 1
  }, {
   "filename": "/Sounds/gna3.wav",
   "start": 182288382,
   "end": 182363752,
   "audio": 1
  }, {
   "filename": "/Sounds/kahe.wav",
   "start": 182363752,
   "end": 182366660,
   "audio": 1
  }, {
   "filename": "/Sounds/ungr.wav",
   "start": 182366660,
   "end": 182376620,
   "audio": 1
  }, {
   "filename": "/Sounds/boh0.wav",
   "start": 182376620,
   "end": 182378736,
   "audio": 1
  }, {
   "filename": "/Sounds/ke_0.wav",
   "start": 182378736,
   "end": 182381924,
   "audio": 1
  }, {
   "filename": "/Sounds/ke_1.wav",
   "start": 182381924,
   "end": 182385118,
   "audio": 1
  }, {
   "filename": "/Sounds/boh1.wav",
   "start": 182385118,
   "end": 182387846,
   "audio": 1
  }, {
   "filename": "/Sounds/hawk.wav",
   "start": 182387846,
   "end": 182809042,
   "audio": 1
  }, {
   "filename": "/Sounds/hat1.wav",
   "start": 182809042,
   "end": 183260938,
   "audio": 1
  }, {
   "filename": "/Sounds/gna2.wav",
   "start": 183260938,
   "end": 183393742,
   "audio": 1
  }, {
   "filename": "/Sounds/foo1.wav",
   "start": 183393742,
   "end": 183396148,
   "audio": 1
  }, {
   "filename": "/Sounds/foop.wav",
   "start": 183396148,
   "end": 183406102,
   "audio": 1
  }, {
   "filename": "/Sounds/recy.wav",
   "start": 183406102,
   "end": 183996260,
   "audio": 1
  }, {
   "filename": "/Sounds/glaf.wav",
   "start": 183996260,
   "end": 184011736,
   "audio": 1
  }, {
   "filename": "/Sounds/poyy.wav",
   "start": 184011736,
   "end": 184120046,
   "audio": 1
  }, {
   "filename": "/Sounds/wav1.wav",
   "start": 184120046,
   "end": 184268826,
   "audio": 1
  }, {
   "filename": "/Sounds/kis1.wav",
   "start": 184268826,
   "end": 184276086,
   "audio": 1
  }, {
   "filename": "/Sounds/brd4.wav",
   "start": 184276086,
   "end": 184330808,
   "audio": 1
  }, {
   "filename": "/Sounds/noh1.wav",
   "start": 184330808,
   "end": 184334648,
   "audio": 1
  }, {
   "filename": "/Sounds/seie.wav",
   "start": 184334648,
   "end": 184338664,
   "audio": 1
  }, {
   "filename": "/Sounds/ber0.wav",
   "start": 184338664,
   "end": 184340804,
   "audio": 1
  }, {
   "filename": "/Sounds/noht.wav",
   "start": 184340804,
   "end": 184347384,
   "audio": 1
  }, {
   "filename": "/Sounds/sc_2.wav",
   "start": 184347384,
   "end": 184380454,
   "audio": 1
  }, {
   "filename": "/Sounds/mb_2.wav",
   "start": 184380454,
   "end": 184474581,
   "audio": 1
  }, {
   "filename": "/Sounds/maan.wav",
   "start": 184474581,
   "end": 184480711,
   "audio": 1
  }, {
   "filename": "/Sounds/cd_1.wav",
   "start": 184480711,
   "end": 184720291,
   "audio": 1
  }, {
   "filename": "/Sounds/undl.wav",
   "start": 184720291,
   "end": 184730863,
   "audio": 1
  }, {
   "filename": "/Sounds/ee_0.wav",
   "start": 184730863,
   "end": 184732465,
   "audio": 1
  }, {
   "filename": "/Sounds/exit.wav",
   "start": 184732465,
   "end": 185924011,
   "audio": 1
  }, {
   "filename": "/Sounds/ee_1.wav",
   "start": 185924011,
   "end": 185925599,
   "audio": 1
  }, {
   "filename": "/Sounds/shut.wav",
   "start": 185925599,
   "end": 186054773,
   "audio": 1
  }, {
   "filename": "/Sounds/gig1.wav",
   "start": 186054773,
   "end": 186065731,
   "audio": 1
  }, {
   "filename": "/Sounds/pl_1.wav",
   "start": 186065731,
   "end": 186099861,
   "audio": 1
  }, {
   "filename": "/Sounds/lg_i.wav",
   "start": 186099861,
   "end": 186111937,
   "audio": 1
  }, {
   "filename": "/Sounds/brd1.wav",
   "start": 186111937,
   "end": 186171059,
   "audio": 1
  }, {
   "filename": "/Sounds/mb_3.wav",
   "start": 186171059,
   "end": 187043561,
   "audio": 1
  }, {
   "filename": "/Sounds/lung.wav",
   "start": 187043561,
   "end": 187114443,
   "audio": 1
  }, {
   "filename": "/Sounds/elaf.wav",
   "start": 187114443,
   "end": 187145322,
   "audio": 1
  }, {
   "filename": "/Sounds/ayee.wav",
   "start": 187145322,
   "end": 187155350,
   "audio": 1
  }, {
   "filename": "/Sounds/ber1.wav",
   "start": 187155350,
   "end": 187157470,
   "audio": 1
  }, {
   "filename": "/Sounds/bere.wav",
   "start": 187157470,
   "end": 187159621,
   "audio": 1
  }, {
   "filename": "/Sounds/dcld.wav",
   "start": 187159621,
   "end": 187473727,
   "audio": 1
  }, {
   "filename": "/Sounds/mb_1.wav",
   "start": 187473727,
   "end": 188066367,
   "audio": 1
  }, {
   "filename": "/Sounds/sc_1.wav",
   "start": 188066367,
   "end": 188093145,
   "audio": 1
  }, {
   "filename": "/Sounds/brd3.wav",
   "start": 188093145,
   "end": 188175207,
   "audio": 1
  }, {
   "filename": "/Sounds/ding.wav",
   "start": 188175207,
   "end": 188197427,
   "audio": 1
  }, {
   "filename": "/Sounds/potn.wav",
   "start": 188197427,
   "end": 188211463,
   "audio": 1
  }, {
   "filename": "/Sounds/desert.mng",
   "start": 188211463,
   "end": 193857907
  }, {
   "filename": "/Sounds/gig3.wav",
   "start": 193857907,
   "end": 193882875,
   "audio": 1
  }, {
   "filename": "/Sounds/boi2.wav",
   "start": 193882875,
   "end": 193896511,
   "audio": 1
  }, {
   "filename": "/Sounds/ee_e.wav",
   "start": 193896511,
   "end": 193898113,
   "audio": 1
  }, {
   "filename": "/Sounds/sce1.wav",
   "start": 193898113,
   "end": 193948909,
   "audio": 1
  }, {
   "filename": "/Sounds/dead.wav",
   "start": 193948909,
   "end": 193964533,
   "audio": 1
  }, {
   "filename": "/Sounds/kohm.wav",
   "start": 193964533,
   "end": 193966733,
   "audio": 1
  }, {
   "filename": "/Sounds/gig2.wav",
   "start": 193966733,
   "end": 193997613,
   "audio": 1
  }, {
   "filename": "/Sounds/pl_2.wav",
   "start": 193997613,
   "end": 194030711,
   "audio": 1
  }, {
   "filename": "/Sounds/brd2.wav",
   "start": 194030711,
   "end": 194130999,
   "audio": 1
  }, {
   "filename": "/Sounds/beem.wav",
   "start": 194130999,
   "end": 194135148,
   "audio": 1
  }, {
   "filename": "/Sounds/beez.wav",
   "start": 194135148,
   "end": 194347936,
   "audio": 1
  }, {
   "filename": "/Sounds/imem.wav",
   "start": 194347936,
   "end": 194356198,
   "audio": 1
  }, {
   "filename": "/Sounds/aiae.wav",
   "start": 194356198,
   "end": 194362576,
   "audio": 1
  }, {
   "filename": "/Backgrounds/crypt1.5.blk",
   "start": 194362576,
   "end": 195509746
  }, {
   "filename": "/Backgrounds/blank.blk",
   "start": 195509746,
   "end": 195903068
  }, {
   "filename": "/Backgrounds/gren3.4.blk",
   "start": 195903068,
   "end": 204555942
  }, {
   "filename": "/Backgrounds/learning_room1.8.blk",
   "start": 204555942,
   "end": 205703112
  }, {
   "filename": "/Backgrounds/pinball2.blk",
   "start": 205703112,
   "end": 206850282
  }, {
   "filename": "/Backgrounds/norn3.0.blk",
   "start": 206850282,
   "end": 218747980
  }, {
   "filename": "/Backgrounds/space.blk",
   "start": 218747980,
   "end": 219895150
  }, {
   "filename": "/Backgrounds/c3_splash.blk",
   "start": 219895150,
   "end": 221042320
  }, {
   "filename": "/Backgrounds/aqua2.6.blk",
   "start": 221042320,
   "end": 228580810
  }, {
   "filename": "/Backgrounds/ettin1.8.blk",
   "start": 228580810,
   "end": 236447060
  }, {
   "filename": "/Backgrounds/main3.0.blk",
   "start": 236447060,
   "end": 259455822
  }, {
   "filename": "/Overlay Data/over_p00a.c16",
   "start": 259455822,
   "end": 259460020
  }, {
   "filename": "/Overlay Data/over_h00a.c16",
   "start": 259460020,
   "end": 259464218
  }, {
   "filename": "/Overlay Data/over_d00a.c16",
   "start": 259464218,
   "end": 259468416
  }, {
   "filename": "/Overlay Data/over_f00a.c16",
   "start": 259468416,
   "end": 259472614
  }, {
   "filename": "/Overlay Data/over_j00a.c16",
   "start": 259472614,
   "end": 259476812
  }, {
   "filename": "/Overlay Data/over_n00a.c16",
   "start": 259476812,
   "end": 259481010
  }, {
   "filename": "/Overlay Data/over_b00a.c16",
   "start": 259481010,
   "end": 259497784
  }, {
   "filename": "/Overlay Data/over_l00a.c16",
   "start": 259497784,
   "end": 259501982
  }, {
   "filename": "/Overlay Data/over_i00a.c16",
   "start": 259501982,
   "end": 259506180
  }, {
   "filename": "/Overlay Data/over_e00a.c16",
   "start": 259506180,
   "end": 259510378
  }, {
   "filename": "/Overlay Data/over_q00a.c16",
   "start": 259510378,
   "end": 259514576
  }, {
   "filename": "/Overlay Data/over_g00a.c16",
   "start": 259514576,
   "end": 259518774
  }, {
   "filename": "/Overlay Data/over_k00a.c16",
   "start": 259518774,
   "end": 259522972
  }, {
   "filename": "/Overlay Data/over_o00a.c16",
   "start": 259522972,
   "end": 259527170
  }, {
   "filename": "/Overlay Data/over_c00a.c16",
   "start": 259527170,
   "end": 259531368
  }, {
   "filename": "/Overlay Data/over_a00a.c16",
   "start": 259531368,
   "end": 259556526
  }, {
   "filename": "/Overlay Data/over_m00a.c16",
   "start": 259556526,
   "end": 259560724
  }, {
   "filename": "/Genetics/norn.civet46.gen.brain.gno",
   "start": 259560724,
   "end": 259693857
  }, {
   "filename": "/Genetics/ettn.final46e.gen.brain.gen",
   "start": 259693857,
   "end": 259729858
  }, {
   "filename": "/Genetics/norn.bengal46.gen.brain.gen",
   "start": 259729858,
   "end": 259765859
  }, {
   "filename": "/Genetics/gren.final46g.gen.brain.gen",
   "start": 259765859,
   "end": 259801860
  }, {
   "filename": "/Genetics/norn.bruin46.gen.brain.gno",
   "start": 259801860,
   "end": 259934993
  }, {
   "filename": "/Genetics/norn.bruin46.gen.brain.gen",
   "start": 259934993,
   "end": 259970994
  }, {
   "filename": "/Genetics/gren.final46g.gen.brain.gno",
   "start": 259970994,
   "end": 260102236
  }, {
   "filename": "/Genetics/norn.bengal46.gen.brain.gno",
   "start": 260102236,
   "end": 260235369
  }, {
   "filename": "/Genetics/norn.civet46.gen.brain.gen",
   "start": 260235369,
   "end": 260271370
  }, {
   "filename": "/Genetics/ettn.final46e.gen.brain.gno",
   "start": 260271370,
   "end": 260404241
  }, {
   "filename": "/Catalogue/Materia Medica-fr.catalogue",
   "start": 260404241,
   "end": 260431533
  }, {
   "filename": "/Catalogue/Creature History-de.catalogue",
   "start": 260431533,
   "end": 260433072
  }, {
   "filename": "/Catalogue/Scrolls of Learning-ru.catalogue",
   "start": 260433072,
   "end": 260439369
  }, {
   "filename": "/Catalogue/Brain-ru.catalogue",
   "start": 260439369,
   "end": 260441504
  }, {
   "filename": "/Catalogue/Scrolls of Learning-fr.catalogue",
   "start": 260441504,
   "end": 260448040
  }, {
   "filename": "/Catalogue/Materia Medica-ru.catalogue",
   "start": 260448040,
   "end": 260470165
  }, {
   "filename": "/Catalogue/GUItext.catalogue",
   "start": 260470165,
   "end": 260471133
  }, {
   "filename": "/Catalogue/Brain-fr.catalogue",
   "start": 260471133,
   "end": 260473842
  }, {
   "filename": "/Catalogue/Norn-it.catalogue",
   "start": 260473842,
   "end": 260477361
  }, {
   "filename": "/Catalogue/Materia Medica-nl.catalogue",
   "start": 260477361,
   "end": 260503330
  }, {
   "filename": "/Catalogue/Materia Medica.catalogue",
   "start": 260503330,
   "end": 260527817
  }, {
   "filename": "/Catalogue/Scrolls of Learning-nl.catalogue",
   "start": 260527817,
   "end": 260534282
  }, {
   "filename": "/Catalogue/Brain-nl.catalogue",
   "start": 260534282,
   "end": 260536883
  }, {
   "filename": "/Catalogue/System-de.catalogue",
   "start": 260536883,
   "end": 260546673
  }, {
   "filename": "/Catalogue/Creatures 3-nl.catalogue",
   "start": 260546673,
   "end": 260552964
  }, {
   "filename": "/Catalogue/Progress-ru.catalogue",
   "start": 260552964,
   "end": 260553425
  }, {
   "filename": "/Catalogue/AgentHelp-it.catalogue",
   "start": 260553425,
   "end": 260636395
  }, {
   "filename": "/Catalogue/Progress-fr.catalogue",
   "start": 260636395,
   "end": 260636859
  }, {
   "filename": "/Catalogue/Brain.catalogue",
   "start": 260636859,
   "end": 260638760
  }, {
   "filename": "/Catalogue/World Switcher-it.catalogue",
   "start": 260638760,
   "end": 260640398
  }, {
   "filename": "/Catalogue/labels-it.catalogue",
   "start": 260640398,
   "end": 260642147
  }, {
   "filename": "/Catalogue/vocab constructs-it.catalogue",
   "start": 260642147,
   "end": 260642769
  }, {
   "filename": "/Catalogue/Creatures 3-fr.catalogue",
   "start": 260642769,
   "end": 260649233
  }, {
   "filename": "/Catalogue/GUItext-es.catalogue",
   "start": 260649233,
   "end": 260650465
  }, {
   "filename": "/Catalogue/Creatures 3-ru.catalogue",
   "start": 260650465,
   "end": 260656519
  }, {
   "filename": "/Catalogue/Progress-nl.catalogue",
   "start": 260656519,
   "end": 260656983
  }, {
   "filename": "/Catalogue/labels-fr.catalogue",
   "start": 260656983,
   "end": 260658880
  }, {
   "filename": "/Catalogue/System-es.catalogue",
   "start": 260658880,
   "end": 260667765
  }, {
   "filename": "/Catalogue/Creatures 3-it.catalogue",
   "start": 260667765,
   "end": 260674089
  }, {
   "filename": "/Catalogue/vocab constructs-fr.catalogue",
   "start": 260674089,
   "end": 260674711
  }, {
   "filename": "/Catalogue/World Switcher-fr.catalogue",
   "start": 260674711,
   "end": 260676464
  }, {
   "filename": "/Catalogue/labels-ru.catalogue",
   "start": 260676464,
   "end": 260677791
  }, {
   "filename": "/Catalogue/vocab constructs-ru.catalogue",
   "start": 260677791,
   "end": 260678421
  }, {
   "filename": "/Catalogue/AgentHelp-nl.catalogue",
   "start": 260678421,
   "end": 260760890
  }, {
   "filename": "/Catalogue/World Switcher-ru.catalogue",
   "start": 260760890,
   "end": 260762093
  }, {
   "filename": "/Catalogue/vocab constructs-nl.catalogue",
   "start": 260762093,
   "end": 260762715
  }, {
   "filename": "/Catalogue/labels-nl.catalogue",
   "start": 260762715,
   "end": 260764552
  }, {
   "filename": "/Catalogue/World Switcher-nl.catalogue",
   "start": 260764552,
   "end": 260766278
  }, {
   "filename": "/Catalogue/AgentHelp-ru.catalogue",
   "start": 260766278,
   "end": 260840119
  }, {
   "filename": "/Catalogue/Progress-it.catalogue",
   "start": 260840119,
   "end": 260840583
  }, {
   "filename": "/Catalogue/GUItext-de.catalogue",
   "start": 260840583,
   "end": 260841750
  }, {
   "filename": "/Catalogue/AgentHelp-fr.catalogue",
   "start": 260841750,
   "end": 260928667
  }, {
   "filename": "/Catalogue/Creature History-es.catalogue",
   "start": 260928667,
   "end": 260930235
  }, {
   "filename": "/Catalogue/AgentHelp.catalogue",
   "start": 260930235,
   "end": 261009891
  }, {
   "filename": "/Catalogue/Norn.catalogue",
   "start": 261009891,
   "end": 261012996
  }, {
   "filename": "/Catalogue/Norn-nl.catalogue",
   "start": 261012996,
   "end": 261016525
  }, {
   "filename": "/Catalogue/Norn-ru.catalogue",
   "start": 261016525,
   "end": 261019848
  }, {
   "filename": "/Catalogue/Patch.catalogue",
   "start": 261019848,
   "end": 261019936
  }, {
   "filename": "/Catalogue/Materia Medica-it.catalogue",
   "start": 261019936,
   "end": 261047692
  }, {
   "filename": "/Catalogue/Brain-it.catalogue",
   "start": 261047692,
   "end": 261050341
  }, {
   "filename": "/Catalogue/Norn-fr.catalogue",
   "start": 261050341,
   "end": 261054013
  }, {
   "filename": "/Catalogue/Creature History.catalogue",
   "start": 261054013,
   "end": 261055153
  }, {
   "filename": "/Catalogue/World Switcher.catalogue",
   "start": 261055153,
   "end": 261056409
  }, {
   "filename": "/Catalogue/Scrolls of Learning-it.catalogue",
   "start": 261056409,
   "end": 261062922
  }, {
   "filename": "/Catalogue/Progress-es.catalogue",
   "start": 261062922,
   "end": 261063386
  }, {
   "filename": "/Catalogue/GUItext-nl.catalogue",
   "start": 261063386,
   "end": 261064638
  }, {
   "filename": "/Catalogue/vocab constructs-de.catalogue",
   "start": 261064638,
   "end": 261065260
  }, {
   "filename": "/Catalogue/labels-de.catalogue",
   "start": 261065260,
   "end": 261067179
  }, {
   "filename": "/Catalogue/World Switcher-de.catalogue",
   "start": 261067179,
   "end": 261068988
  }, {
   "filename": "/Catalogue/AgentHelp-en-GB.catalogue",
   "start": 261068988,
   "end": 261148653
  }, {
   "filename": "/Catalogue/GUItext-fr.catalogue",
   "start": 261148653,
   "end": 261149888
  }, {
   "filename": "/Catalogue/AgentHelp-de.catalogue",
   "start": 261149888,
   "end": 261239722
  }, {
   "filename": "/Catalogue/System-it.catalogue",
   "start": 261239722,
   "end": 261247585
  }, {
   "filename": "/Catalogue/GUItext-ru.catalogue",
   "start": 261247585,
   "end": 261248546
  }, {
   "filename": "/Catalogue/Creatures 3-es.catalogue",
   "start": 261248546,
   "end": 261255017
  }, {
   "filename": "/Catalogue/Scrolls of Learning-es.catalogue",
   "start": 261255017,
   "end": 261261545
  }, {
   "filename": "/Catalogue/Brain-es.catalogue",
   "start": 261261545,
   "end": 261264184
  }, {
   "filename": "/Catalogue/Materia Medica-es.catalogue",
   "start": 261264184,
   "end": 261291809
  }, {
   "filename": "/Catalogue/Scrolls of Learning.catalogue",
   "start": 261291809,
   "end": 261298106
  }, {
   "filename": "/Catalogue/Norn-de.catalogue",
   "start": 261298106,
   "end": 261301656
  }, {
   "filename": "/Catalogue/vocab constructs.catalogue",
   "start": 261301656,
   "end": 261302208
  }, {
   "filename": "/Catalogue/Creature History-it.catalogue",
   "start": 261302208,
   "end": 261303749
  }, {
   "filename": "/Catalogue/Creature History-ru.catalogue",
   "start": 261303749,
   "end": 261304918
  }, {
   "filename": "/Catalogue/Scrolls of Learning-de.catalogue",
   "start": 261304918,
   "end": 261311411
  }, {
   "filename": "/Catalogue/labels.catalogue",
   "start": 261311411,
   "end": 261312718
  }, {
   "filename": "/Catalogue/Brain-de.catalogue",
   "start": 261312718,
   "end": 261315393
  }, {
   "filename": "/Catalogue/Materia Medica-de.catalogue",
   "start": 261315393,
   "end": 261342680
  }, {
   "filename": "/Catalogue/Creature History-fr.catalogue",
   "start": 261342680,
   "end": 261344175
  }, {
   "filename": "/Catalogue/pray_extensions.catalogue",
   "start": 261344175,
   "end": 261344265
  }, {
   "filename": "/Catalogue/Genome.catalogue",
   "start": 261344265,
   "end": 261345041
  }, {
   "filename": "/Catalogue/System.catalogue",
   "start": 261345041,
   "end": 261353253
  }, {
   "filename": "/Catalogue/Norn-es.catalogue",
   "start": 261353253,
   "end": 261356777
  }, {
   "filename": "/Catalogue/ChemicalNames.catalogue",
   "start": 261356777,
   "end": 261360867
  }, {
   "filename": "/Catalogue/Creature History-nl.catalogue",
   "start": 261360867,
   "end": 261362379
  }, {
   "filename": "/Catalogue/Creatures 3.catalogue",
   "start": 261362379,
   "end": 261368330
  }, {
   "filename": "/Catalogue/CAOS.catalogue",
   "start": 261368330,
   "end": 261373957
  }, {
   "filename": "/Catalogue/System-ru.catalogue",
   "start": 261373957,
   "end": 261380929
  }, {
   "filename": "/Catalogue/GUItext-it.catalogue",
   "start": 261380929,
   "end": 261382124
  }, {
   "filename": "/Catalogue/Progress-de.catalogue",
   "start": 261382124,
   "end": 261382588
  }, {
   "filename": "/Catalogue/labels-es.catalogue",
   "start": 261382588,
   "end": 261384597
  }, {
   "filename": "/Catalogue/System-fr.catalogue",
   "start": 261384597,
   "end": 261394680
  }, {
   "filename": "/Catalogue/vocab constructs-es.catalogue",
   "start": 261394680,
   "end": 261395302
  }, {
   "filename": "/Catalogue/World Switcher-es.catalogue",
   "start": 261395302,
   "end": 261397054
  }, {
   "filename": "/Catalogue/AgentHelp-es.catalogue",
   "start": 261397054,
   "end": 261486390
  }, {
   "filename": "/Catalogue/System-nl.catalogue",
   "start": 261486390,
   "end": 261493998
  }, {
   "filename": "/Catalogue/voices.catalogue",
   "start": 261493998,
   "end": 261497195
  }, {
   "filename": "/Catalogue/Creatures 3-de.catalogue",
   "start": 261497195,
   "end": 261503551
  }, {
   "filename": "/Catalogue/Progress.catalogue",
   "start": 261503551,
   "end": 261504012
  } ],
  "remote_package_size": 261504012,
  "package_uuid": "c98937f4-e662-4905-83ea-d68abff197fc"
 });
})();

var necessaryPreJSTasks = Module["preRun"].slice();

if (!Module["preRun"]) throw "Module.preRun should exist because file support used it; did a pre-js delete it?";

necessaryPreJSTasks.forEach(function(task) {
 if (Module["preRun"].indexOf(task) < 0) throw "All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?";
});

var moduleOverrides = {};

var key;

for (key in Module) {
 if (Module.hasOwnProperty(key)) {
  moduleOverrides[key] = Module[key];
 }
}

var arguments_ = [];

var thisProgram = "./this.program";

var quit_ = function(status, toThrow) {
 throw toThrow;
};

var ENVIRONMENT_IS_WEB = typeof window === "object";

var ENVIRONMENT_IS_WORKER = typeof importScripts === "function";

var ENVIRONMENT_IS_NODE = typeof process === "object" && typeof process.versions === "object" && typeof process.versions.node === "string";

var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (Module["ENVIRONMENT"]) {
 throw new Error("Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -s ENVIRONMENT=web or -s ENVIRONMENT=node)");
}

var scriptDirectory = "";

function locateFile(path) {
 if (Module["locateFile"]) {
  return Module["locateFile"](path, scriptDirectory);
 }
 return scriptDirectory + path;
}

var read_, readAsync, readBinary, setWindowTitle;

function logExceptionOnExit(e) {
 if (e instanceof ExitStatus) return;
 var toLog = e;
 if (e && typeof e === "object" && e.stack) {
  toLog = [ e, e.stack ];
 }
 err("exiting due to exception: " + toLog);
}

var nodeFS;

var nodePath;

if (ENVIRONMENT_IS_NODE) {
 if (!(typeof process === "object" && typeof require === "function")) throw new Error("not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)");
 if (ENVIRONMENT_IS_WORKER) {
  scriptDirectory = require("path").dirname(scriptDirectory) + "/";
 } else {
  scriptDirectory = __dirname + "/";
 }
 read_ = function shell_read(filename, binary) {
  if (!nodeFS) nodeFS = require("fs");
  if (!nodePath) nodePath = require("path");
  filename = nodePath["normalize"](filename);
  return nodeFS["readFileSync"](filename, binary ? null : "utf8");
 };
 readBinary = function readBinary(filename) {
  var ret = read_(filename, true);
  if (!ret.buffer) {
   ret = new Uint8Array(ret);
  }
  assert(ret.buffer);
  return ret;
 };
 readAsync = function readAsync(filename, onload, onerror) {
  if (!nodeFS) nodeFS = require("fs");
  if (!nodePath) nodePath = require("path");
  filename = nodePath["normalize"](filename);
  nodeFS["readFile"](filename, function(err, data) {
   if (err) onerror(err); else onload(data.buffer);
  });
 };
 if (process["argv"].length > 1) {
  thisProgram = process["argv"][1].replace(/\\/g, "/");
 }
 arguments_ = process["argv"].slice(2);
 if (typeof module !== "undefined") {
  module["exports"] = Module;
 }
 process["on"]("uncaughtException", function(ex) {
  if (!(ex instanceof ExitStatus)) {
   throw ex;
  }
 });
 process["on"]("unhandledRejection", function(reason) {
  throw reason;
 });
 quit_ = function(status, toThrow) {
  if (keepRuntimeAlive()) {
   process["exitCode"] = status;
   throw toThrow;
  }
  logExceptionOnExit(toThrow);
  process["exit"](status);
 };
 Module["inspect"] = function() {
  return "[Emscripten Module object]";
 };
} else if (ENVIRONMENT_IS_SHELL) {
 if (typeof process === "object" && typeof require === "function" || typeof window === "object" || typeof importScripts === "function") throw new Error("not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)");
 if (typeof read != "undefined") {
  read_ = function shell_read(f) {
   return read(f);
  };
 }
 readBinary = function readBinary(f) {
  var data;
  if (typeof readbuffer === "function") {
   return new Uint8Array(readbuffer(f));
  }
  data = read(f, "binary");
  assert(typeof data === "object");
  return data;
 };
 readAsync = function readAsync(f, onload, onerror) {
  setTimeout(function() {
   onload(readBinary(f));
  }, 0);
 };
 if (typeof scriptArgs != "undefined") {
  arguments_ = scriptArgs;
 } else if (typeof arguments != "undefined") {
  arguments_ = arguments;
 }
 if (typeof quit === "function") {
  quit_ = function(status, toThrow) {
   logExceptionOnExit(toThrow);
   quit(status);
  };
 }
 if (typeof print !== "undefined") {
  if (typeof console === "undefined") console = {};
  console.log = print;
  console.warn = console.error = typeof printErr !== "undefined" ? printErr : print;
 }
} else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
 if (ENVIRONMENT_IS_WORKER) {
  scriptDirectory = self.location.href;
 } else if (typeof document !== "undefined" && document.currentScript) {
  scriptDirectory = document.currentScript.src;
 }
 if (scriptDirectory.indexOf("blob:") !== 0) {
  scriptDirectory = scriptDirectory.substr(0, scriptDirectory.replace(/[?#].*/, "").lastIndexOf("/") + 1);
 } else {
  scriptDirectory = "";
 }
 if (!(typeof window === "object" || typeof importScripts === "function")) throw new Error("not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)");
 {
  read_ = function(url) {
   var xhr = new XMLHttpRequest();
   xhr.open("GET", url, false);
   xhr.send(null);
   return xhr.responseText;
  };
  if (ENVIRONMENT_IS_WORKER) {
   readBinary = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, false);
    xhr.responseType = "arraybuffer";
    xhr.send(null);
    return new Uint8Array(xhr.response);
   };
  }
  readAsync = function(url, onload, onerror) {
   var xhr = new XMLHttpRequest();
   xhr.open("GET", url, true);
   xhr.responseType = "arraybuffer";
   xhr.onload = function() {
    if (xhr.status == 200 || xhr.status == 0 && xhr.response) {
     onload(xhr.response);
     return;
    }
    onerror();
   };
   xhr.onerror = onerror;
   xhr.send(null);
  };
 }
 setWindowTitle = function(title) {
  document.title = title;
 };
} else {
 throw new Error("environment detection error");
}

var out = Module["print"] || console.log.bind(console);

var err = Module["printErr"] || console.warn.bind(console);

for (key in moduleOverrides) {
 if (moduleOverrides.hasOwnProperty(key)) {
  Module[key] = moduleOverrides[key];
 }
}

moduleOverrides = null;

if (Module["arguments"]) arguments_ = Module["arguments"];

if (!Object.getOwnPropertyDescriptor(Module, "arguments")) {
 Object.defineProperty(Module, "arguments", {
  configurable: true,
  get: function() {
   abort("Module.arguments has been replaced with plain arguments_ (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

if (Module["thisProgram"]) thisProgram = Module["thisProgram"];

if (!Object.getOwnPropertyDescriptor(Module, "thisProgram")) {
 Object.defineProperty(Module, "thisProgram", {
  configurable: true,
  get: function() {
   abort("Module.thisProgram has been replaced with plain thisProgram (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

if (Module["quit"]) quit_ = Module["quit"];

if (!Object.getOwnPropertyDescriptor(Module, "quit")) {
 Object.defineProperty(Module, "quit", {
  configurable: true,
  get: function() {
   abort("Module.quit has been replaced with plain quit_ (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

assert(typeof Module["memoryInitializerPrefixURL"] === "undefined", "Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead");

assert(typeof Module["pthreadMainPrefixURL"] === "undefined", "Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead");

assert(typeof Module["cdInitializerPrefixURL"] === "undefined", "Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead");

assert(typeof Module["filePackagePrefixURL"] === "undefined", "Module.filePackagePrefixURL option was removed, use Module.locateFile instead");

assert(typeof Module["read"] === "undefined", "Module.read option was removed (modify read_ in JS)");

assert(typeof Module["readAsync"] === "undefined", "Module.readAsync option was removed (modify readAsync in JS)");

assert(typeof Module["readBinary"] === "undefined", "Module.readBinary option was removed (modify readBinary in JS)");

assert(typeof Module["setWindowTitle"] === "undefined", "Module.setWindowTitle option was removed (modify setWindowTitle in JS)");

assert(typeof Module["TOTAL_MEMORY"] === "undefined", "Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY");

if (!Object.getOwnPropertyDescriptor(Module, "read")) {
 Object.defineProperty(Module, "read", {
  configurable: true,
  get: function() {
   abort("Module.read has been replaced with plain read_ (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

if (!Object.getOwnPropertyDescriptor(Module, "readAsync")) {
 Object.defineProperty(Module, "readAsync", {
  configurable: true,
  get: function() {
   abort("Module.readAsync has been replaced with plain readAsync (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

if (!Object.getOwnPropertyDescriptor(Module, "readBinary")) {
 Object.defineProperty(Module, "readBinary", {
  configurable: true,
  get: function() {
   abort("Module.readBinary has been replaced with plain readBinary (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

if (!Object.getOwnPropertyDescriptor(Module, "setWindowTitle")) {
 Object.defineProperty(Module, "setWindowTitle", {
  configurable: true,
  get: function() {
   abort("Module.setWindowTitle has been replaced with plain setWindowTitle (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

var IDBFS = "IDBFS is no longer included by default; build with -lidbfs.js";

var PROXYFS = "PROXYFS is no longer included by default; build with -lproxyfs.js";

var WORKERFS = "WORKERFS is no longer included by default; build with -lworkerfs.js";

var NODEFS = "NODEFS is no longer included by default; build with -lnodefs.js";

assert(!ENVIRONMENT_IS_SHELL, "shell environment detected but not enabled at build time.  Add 'shell' to `-s ENVIRONMENT` to enable.");

var STACK_ALIGN = 16;

function getNativeTypeSize(type) {
 switch (type) {
 case "i1":
 case "i8":
  return 1;

 case "i16":
  return 2;

 case "i32":
  return 4;

 case "i64":
  return 8;

 case "float":
  return 4;

 case "double":
  return 8;

 default:
  {
   if (type[type.length - 1] === "*") {
    return 4;
   } else if (type[0] === "i") {
    var bits = Number(type.substr(1));
    assert(bits % 8 === 0, "getNativeTypeSize invalid bits " + bits + ", type " + type);
    return bits / 8;
   } else {
    return 0;
   }
  }
 }
}

function warnOnce(text) {
 if (!warnOnce.shown) warnOnce.shown = {};
 if (!warnOnce.shown[text]) {
  warnOnce.shown[text] = 1;
  err(text);
 }
}

function convertJsFunctionToWasm(func, sig) {
 if (typeof WebAssembly.Function === "function") {
  var typeNames = {
   "i": "i32",
   "j": "i64",
   "f": "f32",
   "d": "f64"
  };
  var type = {
   parameters: [],
   results: sig[0] == "v" ? [] : [ typeNames[sig[0]] ]
  };
  for (var i = 1; i < sig.length; ++i) {
   type.parameters.push(typeNames[sig[i]]);
  }
  return new WebAssembly.Function(type, func);
 }
 var typeSection = [ 1, 0, 1, 96 ];
 var sigRet = sig.slice(0, 1);
 var sigParam = sig.slice(1);
 var typeCodes = {
  "i": 127,
  "j": 126,
  "f": 125,
  "d": 124
 };
 typeSection.push(sigParam.length);
 for (var i = 0; i < sigParam.length; ++i) {
  typeSection.push(typeCodes[sigParam[i]]);
 }
 if (sigRet == "v") {
  typeSection.push(0);
 } else {
  typeSection = typeSection.concat([ 1, typeCodes[sigRet] ]);
 }
 typeSection[1] = typeSection.length - 2;
 var bytes = new Uint8Array([ 0, 97, 115, 109, 1, 0, 0, 0 ].concat(typeSection, [ 2, 7, 1, 1, 101, 1, 102, 0, 0, 7, 5, 1, 1, 102, 0, 0 ]));
 var module = new WebAssembly.Module(bytes);
 var instance = new WebAssembly.Instance(module, {
  "e": {
   "f": func
  }
 });
 var wrappedFunc = instance.exports["f"];
 return wrappedFunc;
}

var freeTableIndexes = [];

var functionsInTableMap;

function getEmptyTableSlot() {
 if (freeTableIndexes.length) {
  return freeTableIndexes.pop();
 }
 try {
  wasmTable.grow(1);
 } catch (err) {
  if (!(err instanceof RangeError)) {
   throw err;
  }
  throw "Unable to grow wasm table. Set ALLOW_TABLE_GROWTH.";
 }
 return wasmTable.length - 1;
}

function addFunctionWasm(func, sig) {
 if (!functionsInTableMap) {
  functionsInTableMap = new WeakMap();
  for (var i = 0; i < wasmTable.length; i++) {
   var item = wasmTable.get(i);
   if (item) {
    functionsInTableMap.set(item, i);
   }
  }
 }
 if (functionsInTableMap.has(func)) {
  return functionsInTableMap.get(func);
 }
 var ret = getEmptyTableSlot();
 try {
  wasmTable.set(ret, func);
 } catch (err) {
  if (!(err instanceof TypeError)) {
   throw err;
  }
  assert(typeof sig !== "undefined", "Missing signature argument to addFunction: " + func);
  var wrapped = convertJsFunctionToWasm(func, sig);
  wasmTable.set(ret, wrapped);
 }
 functionsInTableMap.set(func, ret);
 return ret;
}

function removeFunction(index) {
 functionsInTableMap.delete(wasmTable.get(index));
 freeTableIndexes.push(index);
}

function addFunction(func, sig) {
 assert(typeof func !== "undefined");
 return addFunctionWasm(func, sig);
}

var tempRet0 = 0;

var setTempRet0 = function(value) {
 tempRet0 = value;
};

var getTempRet0 = function() {
 return tempRet0;
};

var wasmBinary;

if (Module["wasmBinary"]) wasmBinary = Module["wasmBinary"];

if (!Object.getOwnPropertyDescriptor(Module, "wasmBinary")) {
 Object.defineProperty(Module, "wasmBinary", {
  configurable: true,
  get: function() {
   abort("Module.wasmBinary has been replaced with plain wasmBinary (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

var noExitRuntime = Module["noExitRuntime"] || false;

if (!Object.getOwnPropertyDescriptor(Module, "noExitRuntime")) {
 Object.defineProperty(Module, "noExitRuntime", {
  configurable: true,
  get: function() {
   abort("Module.noExitRuntime has been replaced with plain noExitRuntime (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

if (typeof WebAssembly !== "object") {
 abort("no native wasm support detected");
}

function setValue(ptr, value, type, noSafe) {
 type = type || "i8";
 if (type.charAt(type.length - 1) === "*") type = "i32";
 switch (type) {
 case "i1":
  _asan_js_store_1(ptr >> 0, value);
  break;

 case "i8":
  _asan_js_store_1(ptr >> 0, value);
  break;

 case "i16":
  _asan_js_store_2(ptr >> 1, value);
  break;

 case "i32":
  _asan_js_store_4(ptr >> 2, value);
  break;

 case "i64":
  tempI64 = [ value >>> 0, (tempDouble = value, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
  _asan_js_store_4(ptr >> 2, tempI64[0]), _asan_js_store_4(ptr + 4 >> 2, tempI64[1]);
  break;

 case "float":
  _asan_js_store_f(ptr >> 2, value);
  break;

 case "double":
  _asan_js_store_d(ptr >> 3, value);
  break;

 default:
  abort("invalid type for setValue: " + type);
 }
}

function getValue(ptr, type, noSafe) {
 type = type || "i8";
 if (type.charAt(type.length - 1) === "*") type = "i32";
 switch (type) {
 case "i1":
  return _asan_js_load_1(ptr >> 0);

 case "i8":
  return _asan_js_load_1(ptr >> 0);

 case "i16":
  return _asan_js_load_2(ptr >> 1);

 case "i32":
  return _asan_js_load_4(ptr >> 2);

 case "i64":
  return _asan_js_load_4(ptr >> 2);

 case "float":
  return _asan_js_load_f(ptr >> 2);

 case "double":
  return _asan_js_load_d(ptr >> 3);

 default:
  abort("invalid type for getValue: " + type);
 }
 return null;
}

function _asan_js_load_1(ptr) {
 if (runtimeInitialized) return _asan_c_load_1(ptr);
 return HEAP8[ptr];
}

function _asan_js_load_1u(ptr) {
 if (runtimeInitialized) return _asan_c_load_1u(ptr);
 return HEAPU8[ptr];
}

function _asan_js_load_2(ptr) {
 if (runtimeInitialized) return _asan_c_load_2(ptr);
 return HEAP16[ptr];
}

function _asan_js_load_2u(ptr) {
 if (runtimeInitialized) return _asan_c_load_2u(ptr);
 return HEAPU16[ptr];
}

function _asan_js_load_4(ptr) {
 if (runtimeInitialized) return _asan_c_load_4(ptr);
 return HEAP32[ptr];
}

function _asan_js_load_4u(ptr) {
 if (runtimeInitialized) return _asan_c_load_4u(ptr) >>> 0;
 return HEAPU32[ptr];
}

function _asan_js_load_f(ptr) {
 if (runtimeInitialized) return _asan_c_load_f(ptr);
 return HEAPF32[ptr];
}

function _asan_js_load_d(ptr) {
 if (runtimeInitialized) return _asan_c_load_d(ptr);
 return HEAPF64[ptr];
}

function _asan_js_store_1(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_1(ptr, val);
 return HEAP8[ptr] = val;
}

function _asan_js_store_1u(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_1u(ptr, val);
 return HEAPU8[ptr] = val;
}

function _asan_js_store_2(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_2(ptr, val);
 return HEAP16[ptr] = val;
}

function _asan_js_store_2u(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_2u(ptr, val);
 return HEAPU16[ptr] = val;
}

function _asan_js_store_4(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_4(ptr, val);
 return HEAP32[ptr] = val;
}

function _asan_js_store_4u(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_4u(ptr, val) >>> 0;
 return HEAPU32[ptr] = val;
}

function _asan_js_store_f(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_f(ptr, val);
 return HEAPF32[ptr] = val;
}

function _asan_js_store_d(ptr, val) {
 if (runtimeInitialized) return _asan_c_store_d(ptr, val);
 return HEAPF64[ptr] = val;
}

var wasmMemory;

var ABORT = false;

var EXITSTATUS;

function assert(condition, text) {
 if (!condition) {
  abort("Assertion failed: " + text);
 }
}

function getCFunc(ident) {
 var func = Module["_" + ident];
 assert(func, "Cannot call unknown function " + ident + ", make sure it is exported");
 return func;
}

function ccall(ident, returnType, argTypes, args, opts) {
 var toC = {
  "string": function(str) {
   var ret = 0;
   if (str !== null && str !== undefined && str !== 0) {
    var len = (str.length << 2) + 1;
    ret = stackAlloc(len);
    stringToUTF8(str, ret, len);
   }
   return ret;
  },
  "array": function(arr) {
   var ret = stackAlloc(arr.length);
   writeArrayToMemory(arr, ret);
   return ret;
  }
 };
 function convertReturnValue(ret) {
  if (returnType === "string") return UTF8ToString(ret);
  if (returnType === "boolean") return Boolean(ret);
  return ret;
 }
 var func = getCFunc(ident);
 var cArgs = [];
 var stack = 0;
 assert(returnType !== "array", 'Return type should not be "array".');
 if (args) {
  for (var i = 0; i < args.length; i++) {
   var converter = toC[argTypes[i]];
   if (converter) {
    if (stack === 0) stack = stackSave();
    cArgs[i] = converter(args[i]);
   } else {
    cArgs[i] = args[i];
   }
  }
 }
 var ret = func.apply(null, cArgs);
 function onDone(ret) {
  if (stack !== 0) stackRestore(stack);
  return convertReturnValue(ret);
 }
 ret = onDone(ret);
 return ret;
}

function cwrap(ident, returnType, argTypes, opts) {
 return function() {
  return ccall(ident, returnType, argTypes, arguments, opts);
 };
}

var ALLOC_NORMAL = 0;

var ALLOC_STACK = 1;

function allocate(slab, allocator) {
 var ret;
 assert(typeof allocator === "number", "allocate no longer takes a type argument");
 assert(typeof slab !== "number", "allocate no longer takes a number as arg0");
 if (allocator == ALLOC_STACK) {
  ret = stackAlloc(slab.length);
 } else {
  ret = _malloc(slab.length);
 }
 if (slab.subarray || slab.slice) {
  HEAPU8.set(slab, ret);
 } else {
  HEAPU8.set(new Uint8Array(slab), ret);
 }
 return ret;
}

var UTF8Decoder = typeof TextDecoder !== "undefined" ? new TextDecoder("utf8") : undefined;

function UTF8ArrayToString(heap, idx, maxBytesToRead) {
 var endIdx = idx + maxBytesToRead;
 var endPtr = idx;
 while (heap[endPtr] && !(endPtr >= endIdx)) ++endPtr;
 if (endPtr - idx > 16 && heap.subarray && UTF8Decoder) {
  return UTF8Decoder.decode(heap.subarray(idx, endPtr));
 } else {
  var str = "";
  while (idx < endPtr) {
   var u0 = heap[idx++];
   if (!(u0 & 128)) {
    str += String.fromCharCode(u0);
    continue;
   }
   var u1 = heap[idx++] & 63;
   if ((u0 & 224) == 192) {
    str += String.fromCharCode((u0 & 31) << 6 | u1);
    continue;
   }
   var u2 = heap[idx++] & 63;
   if ((u0 & 240) == 224) {
    u0 = (u0 & 15) << 12 | u1 << 6 | u2;
   } else {
    if ((u0 & 248) != 240) warnOnce("Invalid UTF-8 leading byte 0x" + u0.toString(16) + " encountered when deserializing a UTF-8 string in wasm memory to a JS string!");
    u0 = (u0 & 7) << 18 | u1 << 12 | u2 << 6 | heap[idx++] & 63;
   }
   if (u0 < 65536) {
    str += String.fromCharCode(u0);
   } else {
    var ch = u0 - 65536;
    str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023);
   }
  }
 }
 return str;
}

function UTF8ToString(ptr, maxBytesToRead) {
 return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : "";
}

function stringToUTF8Array(str, heap, outIdx, maxBytesToWrite) {
 if (!(maxBytesToWrite > 0)) return 0;
 var startIdx = outIdx;
 var endIdx = outIdx + maxBytesToWrite - 1;
 for (var i = 0; i < str.length; ++i) {
  var u = str.charCodeAt(i);
  if (u >= 55296 && u <= 57343) {
   var u1 = str.charCodeAt(++i);
   u = 65536 + ((u & 1023) << 10) | u1 & 1023;
  }
  if (u <= 127) {
   if (outIdx >= endIdx) break;
   heap[outIdx++] = u;
  } else if (u <= 2047) {
   if (outIdx + 1 >= endIdx) break;
   heap[outIdx++] = 192 | u >> 6;
   heap[outIdx++] = 128 | u & 63;
  } else if (u <= 65535) {
   if (outIdx + 2 >= endIdx) break;
   heap[outIdx++] = 224 | u >> 12;
   heap[outIdx++] = 128 | u >> 6 & 63;
   heap[outIdx++] = 128 | u & 63;
  } else {
   if (outIdx + 3 >= endIdx) break;
   if (u > 1114111) warnOnce("Invalid Unicode code point 0x" + u.toString(16) + " encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).");
   heap[outIdx++] = 240 | u >> 18;
   heap[outIdx++] = 128 | u >> 12 & 63;
   heap[outIdx++] = 128 | u >> 6 & 63;
   heap[outIdx++] = 128 | u & 63;
  }
 }
 heap[outIdx] = 0;
 return outIdx - startIdx;
}

function stringToUTF8(str, outPtr, maxBytesToWrite) {
 assert(typeof maxBytesToWrite == "number", "stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!");
 return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
}

function lengthBytesUTF8(str) {
 var len = 0;
 for (var i = 0; i < str.length; ++i) {
  var u = str.charCodeAt(i);
  if (u >= 55296 && u <= 57343) u = 65536 + ((u & 1023) << 10) | str.charCodeAt(++i) & 1023;
  if (u <= 127) ++len; else if (u <= 2047) len += 2; else if (u <= 65535) len += 3; else len += 4;
 }
 return len;
}

function AsciiToString(ptr) {
 var str = "";
 while (1) {
  var ch = _asan_js_load_1u(ptr++ >> 0);
  if (!ch) return str;
  str += String.fromCharCode(ch);
 }
}

function stringToAscii(str, outPtr) {
 return writeAsciiToMemory(str, outPtr, false);
}

var UTF16Decoder = typeof TextDecoder !== "undefined" ? new TextDecoder("utf-16le") : undefined;

function UTF16ToString(ptr, maxBytesToRead) {
 assert(ptr % 2 == 0, "Pointer passed to UTF16ToString must be aligned to two bytes!");
 var endPtr = ptr;
 var idx = endPtr >> 1;
 var maxIdx = idx + maxBytesToRead / 2;
 while (!(idx >= maxIdx) && _asan_js_load_2u(idx)) ++idx;
 endPtr = idx << 1;
 if (endPtr - ptr > 32 && UTF16Decoder) {
  return UTF16Decoder.decode(HEAPU8.subarray(ptr, endPtr));
 } else {
  var str = "";
  for (var i = 0; !(i >= maxBytesToRead / 2); ++i) {
   var codeUnit = _asan_js_load_2(ptr + i * 2 >> 1);
   if (codeUnit == 0) break;
   str += String.fromCharCode(codeUnit);
  }
  return str;
 }
}

function stringToUTF16(str, outPtr, maxBytesToWrite) {
 assert(outPtr % 2 == 0, "Pointer passed to stringToUTF16 must be aligned to two bytes!");
 assert(typeof maxBytesToWrite == "number", "stringToUTF16(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!");
 if (maxBytesToWrite === undefined) {
  maxBytesToWrite = 2147483647;
 }
 if (maxBytesToWrite < 2) return 0;
 maxBytesToWrite -= 2;
 var startPtr = outPtr;
 var numCharsToWrite = maxBytesToWrite < str.length * 2 ? maxBytesToWrite / 2 : str.length;
 for (var i = 0; i < numCharsToWrite; ++i) {
  var codeUnit = str.charCodeAt(i);
  _asan_js_store_2(outPtr >> 1, codeUnit);
  outPtr += 2;
 }
 _asan_js_store_2(outPtr >> 1, 0);
 return outPtr - startPtr;
}

function lengthBytesUTF16(str) {
 return str.length * 2;
}

function UTF32ToString(ptr, maxBytesToRead) {
 assert(ptr % 4 == 0, "Pointer passed to UTF32ToString must be aligned to four bytes!");
 var i = 0;
 var str = "";
 while (!(i >= maxBytesToRead / 4)) {
  var utf32 = _asan_js_load_4(ptr + i * 4 >> 2);
  if (utf32 == 0) break;
  ++i;
  if (utf32 >= 65536) {
   var ch = utf32 - 65536;
   str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023);
  } else {
   str += String.fromCharCode(utf32);
  }
 }
 return str;
}

function stringToUTF32(str, outPtr, maxBytesToWrite) {
 assert(outPtr % 4 == 0, "Pointer passed to stringToUTF32 must be aligned to four bytes!");
 assert(typeof maxBytesToWrite == "number", "stringToUTF32(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!");
 if (maxBytesToWrite === undefined) {
  maxBytesToWrite = 2147483647;
 }
 if (maxBytesToWrite < 4) return 0;
 var startPtr = outPtr;
 var endPtr = startPtr + maxBytesToWrite - 4;
 for (var i = 0; i < str.length; ++i) {
  var codeUnit = str.charCodeAt(i);
  if (codeUnit >= 55296 && codeUnit <= 57343) {
   var trailSurrogate = str.charCodeAt(++i);
   codeUnit = 65536 + ((codeUnit & 1023) << 10) | trailSurrogate & 1023;
  }
  _asan_js_store_4(outPtr >> 2, codeUnit);
  outPtr += 4;
  if (outPtr + 4 > endPtr) break;
 }
 _asan_js_store_4(outPtr >> 2, 0);
 return outPtr - startPtr;
}

function lengthBytesUTF32(str) {
 var len = 0;
 for (var i = 0; i < str.length; ++i) {
  var codeUnit = str.charCodeAt(i);
  if (codeUnit >= 55296 && codeUnit <= 57343) ++i;
  len += 4;
 }
 return len;
}

function allocateUTF8(str) {
 var size = lengthBytesUTF8(str) + 1;
 var ret = _malloc(size);
 if (ret) stringToUTF8Array(str, HEAP8, ret, size);
 return ret;
}

function allocateUTF8OnStack(str) {
 var size = lengthBytesUTF8(str) + 1;
 var ret = stackAlloc(size);
 stringToUTF8Array(str, HEAP8, ret, size);
 return ret;
}

function writeStringToMemory(string, buffer, dontAddNull) {
 warnOnce("writeStringToMemory is deprecated and should not be called! Use stringToUTF8() instead!");
 var lastChar, end;
 if (dontAddNull) {
  end = buffer + lengthBytesUTF8(string);
  lastChar = _asan_js_load_1(end);
 }
 stringToUTF8(string, buffer, Infinity);
 if (dontAddNull) _asan_js_store_1(end, lastChar);
}

function writeArrayToMemory(array, buffer) {
 assert(array.length >= 0, "writeArrayToMemory array must have a length (should be an array or typed array)");
 HEAP8.set(array, buffer);
}

function writeAsciiToMemory(str, buffer, dontAddNull) {
 for (var i = 0; i < str.length; ++i) {
  assert(str.charCodeAt(i) === str.charCodeAt(i) & 255);
  _asan_js_store_1(buffer++ >> 0, str.charCodeAt(i));
 }
 if (!dontAddNull) _asan_js_store_1(buffer >> 0, 0);
}

function alignUp(x, multiple) {
 if (x % multiple > 0) {
  x += multiple - x % multiple;
 }
 return x;
}

var HEAP, buffer, HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;

function updateGlobalBufferAndViews(buf) {
 buffer = buf;
 Module["HEAP8"] = HEAP8 = new Int8Array(buf);
 Module["HEAP16"] = HEAP16 = new Int16Array(buf);
 Module["HEAP32"] = HEAP32 = new Int32Array(buf);
 Module["HEAPU8"] = HEAPU8 = new Uint8Array(buf);
 Module["HEAPU16"] = HEAPU16 = new Uint16Array(buf);
 Module["HEAPU32"] = HEAPU32 = new Uint32Array(buf);
 Module["HEAPF32"] = HEAPF32 = new Float32Array(buf);
 Module["HEAPF64"] = HEAPF64 = new Float64Array(buf);
}

var TOTAL_STACK = 5242880;

if (Module["TOTAL_STACK"]) assert(TOTAL_STACK === Module["TOTAL_STACK"], "the stack size can no longer be determined at runtime");

var INITIAL_MEMORY = Module["INITIAL_MEMORY"] || 308871168;

if (!Object.getOwnPropertyDescriptor(Module, "INITIAL_MEMORY")) {
 Object.defineProperty(Module, "INITIAL_MEMORY", {
  configurable: true,
  get: function() {
   abort("Module.INITIAL_MEMORY has been replaced with plain INITIAL_MEMORY (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)");
  }
 });
}

assert(INITIAL_MEMORY >= TOTAL_STACK, "INITIAL_MEMORY should be larger than TOTAL_STACK, was " + INITIAL_MEMORY + "! (TOTAL_STACK=" + TOTAL_STACK + ")");

assert(typeof Int32Array !== "undefined" && typeof Float64Array !== "undefined" && Int32Array.prototype.subarray !== undefined && Int32Array.prototype.set !== undefined, "JS engine does not provide full typed array support");

assert(!Module["wasmMemory"], "Use of `wasmMemory` detected.  Use -s IMPORTED_MEMORY to define wasmMemory externally");

assert(INITIAL_MEMORY == 308871168, "Detected runtime INITIAL_MEMORY setting.  Use -s IMPORTED_MEMORY to define wasmMemory dynamically");

var wasmTable;

function writeStackCookie() {
 var max = _emscripten_stack_get_end();
 assert((max & 3) == 0);
 _asan_js_store_4u((max >> 2) + 1, 34821223);
 _asan_js_store_4u((max >> 2) + 2, 2310721022);
}

function checkStackCookie() {
 if (ABORT) return;
 var max = _emscripten_stack_get_end();
 var cookie1 = _asan_js_load_4u((max >> 2) + 1);
 var cookie2 = _asan_js_load_4u((max >> 2) + 2);
 if (cookie1 != 34821223 || cookie2 != 2310721022) {
  abort("Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x2135467, but received 0x" + cookie2.toString(16) + " " + cookie1.toString(16));
 }
}

(function() {
 var h16 = new Int16Array(1);
 var h8 = new Int8Array(h16.buffer);
 h16[0] = 25459;
 if (h8[0] !== 115 || h8[1] !== 99) throw "Runtime error: expected the system to be little-endian! (Run with -s SUPPORT_BIG_ENDIAN=1 to bypass)";
})();

var __ATPRERUN__ = [];

var __ATINIT__ = [];

var __ATMAIN__ = [];

var __ATEXIT__ = [];

var __ATPOSTRUN__ = [];

var runtimeInitialized = false;

var runtimeExited = false;

var runtimeKeepaliveCounter = 0;

function keepRuntimeAlive() {
 return noExitRuntime || runtimeKeepaliveCounter > 0;
}

function preRun() {
 if (Module["preRun"]) {
  if (typeof Module["preRun"] == "function") Module["preRun"] = [ Module["preRun"] ];
  while (Module["preRun"].length) {
   addOnPreRun(Module["preRun"].shift());
  }
 }
 callRuntimeCallbacks(__ATPRERUN__);
}

function initRuntime() {
 checkStackCookie();
 assert(!runtimeInitialized);
 runtimeInitialized = true;
 if (!Module["noFSInit"] && !FS.init.initialized) FS.init();
 FS.ignorePermissions = false;
 TTY.init();
 SOCKFS.root = FS.mount(SOCKFS, {}, null);
 callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
 checkStackCookie();
 callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
 checkStackCookie();
 callRuntimeCallbacks(__ATEXIT__);
 FS.quit();
 TTY.shutdown();
 runtimeExited = true;
}

function postRun() {
 checkStackCookie();
 if (Module["postRun"]) {
  if (typeof Module["postRun"] == "function") Module["postRun"] = [ Module["postRun"] ];
  while (Module["postRun"].length) {
   addOnPostRun(Module["postRun"].shift());
  }
 }
 callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
 __ATPRERUN__.unshift(cb);
}

function addOnInit(cb) {
 __ATINIT__.unshift(cb);
}

function addOnPreMain(cb) {
 __ATMAIN__.unshift(cb);
}

function addOnExit(cb) {
 __ATEXIT__.unshift(cb);
}

function addOnPostRun(cb) {
 __ATPOSTRUN__.unshift(cb);
}

assert(Math.imul, "This browser does not support Math.imul(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");

assert(Math.fround, "This browser does not support Math.fround(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");

assert(Math.clz32, "This browser does not support Math.clz32(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");

assert(Math.trunc, "This browser does not support Math.trunc(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");

var runDependencies = 0;

var runDependencyWatcher = null;

var dependenciesFulfilled = null;

var runDependencyTracking = {};

function getUniqueRunDependency(id) {
 var orig = id;
 while (1) {
  if (!runDependencyTracking[id]) return id;
  id = orig + Math.random();
 }
}

function addRunDependency(id) {
 runDependencies++;
 if (Module["monitorRunDependencies"]) {
  Module["monitorRunDependencies"](runDependencies);
 }
 if (id) {
  assert(!runDependencyTracking[id]);
  runDependencyTracking[id] = 1;
  if (runDependencyWatcher === null && typeof setInterval !== "undefined") {
   runDependencyWatcher = setInterval(function() {
    if (ABORT) {
     clearInterval(runDependencyWatcher);
     runDependencyWatcher = null;
     return;
    }
    var shown = false;
    for (var dep in runDependencyTracking) {
     if (!shown) {
      shown = true;
      err("still waiting on run dependencies:");
     }
     err("dependency: " + dep);
    }
    if (shown) {
     err("(end of list)");
    }
   }, 1e4);
  }
 } else {
  err("warning: run dependency added without ID");
 }
}

function removeRunDependency(id) {
 runDependencies--;
 if (Module["monitorRunDependencies"]) {
  Module["monitorRunDependencies"](runDependencies);
 }
 if (id) {
  assert(runDependencyTracking[id]);
  delete runDependencyTracking[id];
 } else {
  err("warning: run dependency removed without ID");
 }
 if (runDependencies == 0) {
  if (runDependencyWatcher !== null) {
   clearInterval(runDependencyWatcher);
   runDependencyWatcher = null;
  }
  if (dependenciesFulfilled) {
   var callback = dependenciesFulfilled;
   dependenciesFulfilled = null;
   callback();
  }
 }
}

Module["preloadedImages"] = {};

Module["preloadedAudios"] = {};

function abort(what) {
 {
  if (Module["onAbort"]) {
   Module["onAbort"](what);
  }
 }
 what = "Aborted(" + what + ")";
 err(what);
 ABORT = true;
 EXITSTATUS = 1;
 var e = new WebAssembly.RuntimeError(what);
 throw e;
}

var dataURIPrefix = "data:application/octet-stream;base64,";

function isDataURI(filename) {
 return filename.startsWith(dataURIPrefix);
}

function isFileURI(filename) {
 return filename.startsWith("file://");
}

function createExportWrapper(name, fixedasm) {
 return function() {
  var displayName = name;
  var asm = fixedasm;
  if (!fixedasm) {
   asm = Module["asm"];
  }
  assert(runtimeInitialized, "native function `" + displayName + "` called before runtime initialization");
  assert(!runtimeExited, "native function `" + displayName + "` called after runtime exit (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  if (!asm[name]) {
   assert(asm[name], "exported native function `" + displayName + "` not found");
  }
  return asm[name].apply(null, arguments);
 };
}

var wasmBinaryFile;

wasmBinaryFile = "webc2e.wasm";

if (!isDataURI(wasmBinaryFile)) {
 wasmBinaryFile = locateFile(wasmBinaryFile);
}

function getBinary(file) {
 try {
  if (file == wasmBinaryFile && wasmBinary) {
   return new Uint8Array(wasmBinary);
  }
  if (readBinary) {
   return readBinary(file);
  } else {
   throw "both async and sync fetching of the wasm failed";
  }
 } catch (err) {
  abort(err);
 }
}

function getBinaryPromise() {
 if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
  if (typeof fetch === "function" && !isFileURI(wasmBinaryFile)) {
   return fetch(wasmBinaryFile, {
    credentials: "same-origin"
   }).then(function(response) {
    if (!response["ok"]) {
     throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
    }
    return response["arrayBuffer"]();
   }).catch(function() {
    return getBinary(wasmBinaryFile);
   });
  } else {
   if (readAsync) {
    return new Promise(function(resolve, reject) {
     readAsync(wasmBinaryFile, function(response) {
      resolve(new Uint8Array(response));
     }, reject);
    });
   }
  }
 }
 return Promise.resolve().then(function() {
  return getBinary(wasmBinaryFile);
 });
}

var wasmOffsetConverter;

function WasmOffsetConverter(wasmBytes, wasmModule) {
 var offset = 8;
 var funcidx = 0;
 this.offset_map = {};
 this.func_starts = [];
 this.name_map = {};
 this.import_functions = 0;
 var buffer = wasmBytes;
 function unsignedLEB128() {
  var result = 0;
  var shift = 0;
  do {
   var byte = buffer[offset++];
   result += (byte & 127) << shift;
   shift += 7;
  } while (byte & 128);
  return result;
 }
 function skipLimits() {
  var flags = unsignedLEB128();
  unsignedLEB128();
  var hasMax = (flags & 1) != 0;
  if (hasMax) {
   unsignedLEB128();
  }
 }
 binary_parse: while (offset < buffer.length) {
  var start = offset;
  var type = buffer[offset++];
  var end = unsignedLEB128() + offset;
  switch (type) {
  case 2:
   var count = unsignedLEB128();
   while (count-- > 0) {
    offset = unsignedLEB128() + offset;
    offset = unsignedLEB128() + offset;
    switch (buffer[offset++]) {
    case 0:
     ++funcidx;
     unsignedLEB128();
     break;

    case 1:
     ++offset;
     skipLimits();
     break;

    case 2:
     skipLimits();
     break;

    case 3:
     offset += 2;
     break;

    default:
     throw "bad import kind";
    }
   }
   this.import_functions = funcidx;
   break;

  case 10:
   var count = unsignedLEB128();
   while (count-- > 0) {
    var size = unsignedLEB128();
    this.offset_map[funcidx++] = offset;
    this.func_starts.push(offset);
    offset += size;
   }
   break binary_parse;
  }
  offset = end;
 }
 var sections = WebAssembly.Module.customSections(wasmModule, "name");
 for (var i = 0; i < sections.length; ++i) {
  buffer = new Uint8Array(sections[i]);
  if (buffer[0] != 1) continue;
  offset = 1;
  unsignedLEB128();
  var count = unsignedLEB128();
  while (count-- > 0) {
   var index = unsignedLEB128();
   var length = unsignedLEB128();
   this.name_map[index] = UTF8ArrayToString(buffer, offset, length);
   offset += length;
  }
 }
}

WasmOffsetConverter.prototype.convert = function(funcidx, offset) {
 return this.offset_map[funcidx] + offset;
};

WasmOffsetConverter.prototype.getIndex = function(offset) {
 var lo = 0;
 var hi = this.func_starts.length;
 var mid;
 while (lo < hi) {
  mid = Math.floor((lo + hi) / 2);
  if (this.func_starts[mid] > offset) {
   hi = mid;
  } else {
   lo = mid + 1;
  }
 }
 return lo + this.import_functions - 1;
};

WasmOffsetConverter.prototype.isSameFunc = function(offset1, offset2) {
 return this.getIndex(offset1) == this.getIndex(offset2);
};

WasmOffsetConverter.prototype.getName = function(offset) {
 var index = this.getIndex(offset);
 return this.name_map[index] || "wasm-function[" + index + "]";
};

function resetPrototype(constructor, attrs) {
 var object = Object.create(constructor.prototype);
 for (var key in attrs) {
  if (attrs.hasOwnProperty(key)) {
   object[key] = attrs[key];
  }
 }
 return object;
}

function createWasm() {
 var info = {
  "env": asmLibraryArg,
  "wasi_snapshot_preview1": asmLibraryArg
 };
 function receiveInstance(instance, module) {
  var exports = instance.exports;
  Module["asm"] = exports;
  wasmMemory = Module["asm"]["memory"];
  assert(wasmMemory, "memory not found in wasm exports");
  updateGlobalBufferAndViews(wasmMemory.buffer);
  wasmTable = Module["asm"]["__indirect_function_table"];
  assert(wasmTable, "table not found in wasm exports");
  addOnInit(Module["asm"]["__wasm_call_ctors"]);
  removeRunDependency("wasm-instantiate");
 }
 addRunDependency("wasm-instantiate");
 var trueModule = Module;
 function receiveInstantiationResult(result) {
  assert(Module === trueModule, "the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?");
  trueModule = null;
  receiveInstance(result["instance"]);
 }
 function instantiateArrayBuffer(receiver) {
  var savedBinary;
  return getBinaryPromise().then(function(binary) {
   savedBinary = binary;
   return WebAssembly.instantiate(binary, info);
  }).then(function(instance) {
   wasmOffsetConverter = new WasmOffsetConverter(savedBinary, instance.module);
   return instance;
  }).then(receiver, function(reason) {
   err("failed to asynchronously prepare wasm: " + reason);
   if (isFileURI(wasmBinaryFile)) {
    err("warning: Loading from a file URI (" + wasmBinaryFile + ") is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing");
   }
   abort(reason);
  });
 }
 function instantiateAsync() {
  if (!wasmBinary && typeof WebAssembly.instantiateStreaming === "function" && !isDataURI(wasmBinaryFile) && !isFileURI(wasmBinaryFile) && typeof fetch === "function") {
   return fetch(wasmBinaryFile, {
    credentials: "same-origin"
   }).then(function(response) {
    var result = WebAssembly.instantiateStreaming(response, info);
    var clonedResponsePromise = response.clone().arrayBuffer();
    return result.then(function(instantiationResult) {
     clonedResponsePromise.then(function(arrayBufferResult) {
      wasmOffsetConverter = new WasmOffsetConverter(new Uint8Array(arrayBufferResult), instantiationResult.module);
      receiveInstantiationResult(instantiationResult);
     }, function(reason) {
      err("failed to initialize offset-converter: " + reason);
     });
    }, function(reason) {
     err("wasm streaming compile failed: " + reason);
     err("falling back to ArrayBuffer instantiation");
     return instantiateArrayBuffer(receiveInstantiationResult);
    });
   });
  } else {
   return instantiateArrayBuffer(receiveInstantiationResult);
  }
 }
 if (Module["instantiateWasm"]) {
  wasmOffsetConverter = resetPrototype(WasmOffsetConverter, Module["wasmOffsetData"]);
  try {
   var exports = Module["instantiateWasm"](info, receiveInstance);
   return exports;
  } catch (e) {
   err("Module.instantiateWasm callback failed with error: " + e);
   return false;
  }
 }
 instantiateAsync();
 return {};
}

var tempDouble;

var tempI64;

var ASM_CONSTS = {
 269583168: function() {
  Module.logReadFiles = "true";
 },
 269583202: function() {},
 269583203: function() {},
 269583204: function() {},
 269583205: function() {},
 269583206: function() {},
 269583207: function() {},
 269583208: function() {},
 269583209: function() {},
 269583210: function() {},
 269583211: function() {},
 269583212: function() {},
 269583213: function() {},
 269583214: function() {},
 269583215: function() {},
 269583216: function() {},
 269583217: function() {},
 269583218: function() {},
 269583219: function() {},
 269583220: function() {},
 269583221: function() {},
 269583222: function() {},
 269583223: function() {},
 269583224: function() {},
 269583225: function() {},
 269583226: function() {},
 269583227: function() {},
 269583228: function() {},
 269583229: function() {},
 269583230: function() {},
 269583231: function() {},
 269583232: function() {},
 269583233: function() {},
 269583234: function() {},
 269583235: function() {},
 269583236: function() {},
 269583237: function() {},
 269583238: function() {},
 269583239: function() {},
 269583240: function() {},
 269583241: function() {},
 269583242: function() {},
 269583243: function() {},
 269583244: function() {},
 269583245: function() {},
 269583246: function() {},
 269583247: function() {},
 269583248: function() {},
 269583249: function() {},
 269583250: function() {},
 269583251: function() {},
 269583252: function() {},
 269583253: function() {},
 269583254: function() {},
 269583255: function() {},
 269583256: function() {},
 269583257: function() {},
 269583258: function() {},
 269583259: function() {},
 269583260: function() {},
 269583261: function() {},
 269583262: function() {},
 269583263: function() {},
 269583264: function() {
  return withBuiltinMalloc(function() {
   return allocateUTF8(Module["ASAN_OPTIONS"] || 0);
  });
 },
 269583361: function() {
  return withBuiltinMalloc(function() {
   return allocateUTF8(Module["LSAN_OPTIONS"] || 0);
  });
 },
 269583458: function() {
  return withBuiltinMalloc(function() {
   return allocateUTF8(Module["UBSAN_OPTIONS"] || 0);
  });
 },
 269583556: function() {
  var setting = Module["printWithColors"];
  if (setting != null) {
   return setting;
  } else {
   return ENVIRONMENT_IS_NODE && process.stderr.isTTY;
  }
 }
};

function callRuntimeCallbacks(callbacks) {
 while (callbacks.length > 0) {
  var callback = callbacks.shift();
  if (typeof callback == "function") {
   callback(Module);
   continue;
  }
  var func = callback.func;
  if (typeof func === "number") {
   if (callback.arg === undefined) {
    wasmTable.get(func)();
   } else {
    wasmTable.get(func)(callback.arg);
   }
  } else {
   func(callback.arg === undefined ? null : callback.arg);
  }
 }
}

function demangle(func) {
 warnOnce("warning: build with  -s DEMANGLE_SUPPORT=1  to link in libcxxabi demangling");
 return func;
}

function demangleAll(text) {
 var regex = /\b_Z[\w\d_]+/g;
 return text.replace(regex, function(x) {
  var y = demangle(x);
  return x === y ? x : y + " [" + x + "]";
 });
}

function handleException(e) {
 if (e instanceof ExitStatus || e == "unwind") {
  return EXITSTATUS;
 }
 quit_(1, e);
}

function jsStackTrace() {
 var error = new Error();
 if (!error.stack) {
  try {
   throw new Error();
  } catch (e) {
   error = e;
  }
  if (!error.stack) {
   return "(no stack trace available)";
  }
 }
 return error.stack.toString();
}

function stackTrace() {
 var js = jsStackTrace();
 if (Module["extraStackTrace"]) js += "\n" + Module["extraStackTrace"]();
 return demangleAll(js);
}

function getRandomDevice() {
 if (typeof crypto === "object" && typeof crypto["getRandomValues"] === "function") {
  var randomBuffer = new Uint8Array(1);
  return function() {
   crypto.getRandomValues(randomBuffer);
   return randomBuffer[0];
  };
 } else if (ENVIRONMENT_IS_NODE) {
  try {
   var crypto_module = require("crypto");
   return function() {
    return crypto_module["randomBytes"](1)[0];
   };
  } catch (e) {}
 }
 return function() {
  abort("no cryptographic support found for randomDevice. consider polyfilling it if you want to use something insecure like Math.random(), e.g. put this in a --pre-js: var crypto = { getRandomValues: function(array) { for (var i = 0; i < array.length; i++) array[i] = (Math.random()*256)|0 } };");
 };
}

var PATH = {
 splitPath: function(filename) {
  var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
  return splitPathRe.exec(filename).slice(1);
 },
 normalizeArray: function(parts, allowAboveRoot) {
  var up = 0;
  for (var i = parts.length - 1; i >= 0; i--) {
   var last = parts[i];
   if (last === ".") {
    parts.splice(i, 1);
   } else if (last === "..") {
    parts.splice(i, 1);
    up++;
   } else if (up) {
    parts.splice(i, 1);
    up--;
   }
  }
  if (allowAboveRoot) {
   for (;up; up--) {
    parts.unshift("..");
   }
  }
  return parts;
 },
 normalize: function(path) {
  var isAbsolute = path.charAt(0) === "/", trailingSlash = path.substr(-1) === "/";
  path = PATH.normalizeArray(path.split("/").filter(function(p) {
   return !!p;
  }), !isAbsolute).join("/");
  if (!path && !isAbsolute) {
   path = ".";
  }
  if (path && trailingSlash) {
   path += "/";
  }
  return (isAbsolute ? "/" : "") + path;
 },
 dirname: function(path) {
  var result = PATH.splitPath(path), root = result[0], dir = result[1];
  if (!root && !dir) {
   return ".";
  }
  if (dir) {
   dir = dir.substr(0, dir.length - 1);
  }
  return root + dir;
 },
 basename: function(path) {
  if (path === "/") return "/";
  path = PATH.normalize(path);
  path = path.replace(/\/$/, "");
  var lastSlash = path.lastIndexOf("/");
  if (lastSlash === -1) return path;
  return path.substr(lastSlash + 1);
 },
 extname: function(path) {
  return PATH.splitPath(path)[3];
 },
 join: function() {
  var paths = Array.prototype.slice.call(arguments, 0);
  return PATH.normalize(paths.join("/"));
 },
 join2: function(l, r) {
  return PATH.normalize(l + "/" + r);
 }
};

var PATH_FS = {
 resolve: function() {
  var resolvedPath = "", resolvedAbsolute = false;
  for (var i = arguments.length - 1; i >= -1 && !resolvedAbsolute; i--) {
   var path = i >= 0 ? arguments[i] : FS.cwd();
   if (typeof path !== "string") {
    throw new TypeError("Arguments to path.resolve must be strings");
   } else if (!path) {
    return "";
   }
   resolvedPath = path + "/" + resolvedPath;
   resolvedAbsolute = path.charAt(0) === "/";
  }
  resolvedPath = PATH.normalizeArray(resolvedPath.split("/").filter(function(p) {
   return !!p;
  }), !resolvedAbsolute).join("/");
  return (resolvedAbsolute ? "/" : "") + resolvedPath || ".";
 },
 relative: function(from, to) {
  from = PATH_FS.resolve(from).substr(1);
  to = PATH_FS.resolve(to).substr(1);
  function trim(arr) {
   var start = 0;
   for (;start < arr.length; start++) {
    if (arr[start] !== "") break;
   }
   var end = arr.length - 1;
   for (;end >= 0; end--) {
    if (arr[end] !== "") break;
   }
   if (start > end) return [];
   return arr.slice(start, end - start + 1);
  }
  var fromParts = trim(from.split("/"));
  var toParts = trim(to.split("/"));
  var length = Math.min(fromParts.length, toParts.length);
  var samePartsLength = length;
  for (var i = 0; i < length; i++) {
   if (fromParts[i] !== toParts[i]) {
    samePartsLength = i;
    break;
   }
  }
  var outputParts = [];
  for (var i = samePartsLength; i < fromParts.length; i++) {
   outputParts.push("..");
  }
  outputParts = outputParts.concat(toParts.slice(samePartsLength));
  return outputParts.join("/");
 }
};

var TTY = {
 ttys: [],
 init: function() {},
 shutdown: function() {},
 register: function(dev, ops) {
  TTY.ttys[dev] = {
   input: [],
   output: [],
   ops: ops
  };
  FS.registerDevice(dev, TTY.stream_ops);
 },
 stream_ops: {
  open: function(stream) {
   var tty = TTY.ttys[stream.node.rdev];
   if (!tty) {
    throw new FS.ErrnoError(43);
   }
   stream.tty = tty;
   stream.seekable = false;
  },
  close: function(stream) {
   stream.tty.ops.flush(stream.tty);
  },
  flush: function(stream) {
   stream.tty.ops.flush(stream.tty);
  },
  read: function(stream, buffer, offset, length, pos) {
   if (!stream.tty || !stream.tty.ops.get_char) {
    throw new FS.ErrnoError(60);
   }
   var bytesRead = 0;
   for (var i = 0; i < length; i++) {
    var result;
    try {
     result = stream.tty.ops.get_char(stream.tty);
    } catch (e) {
     throw new FS.ErrnoError(29);
    }
    if (result === undefined && bytesRead === 0) {
     throw new FS.ErrnoError(6);
    }
    if (result === null || result === undefined) break;
    bytesRead++;
    buffer[offset + i] = result;
   }
   if (bytesRead) {
    stream.node.timestamp = Date.now();
   }
   return bytesRead;
  },
  write: function(stream, buffer, offset, length, pos) {
   if (!stream.tty || !stream.tty.ops.put_char) {
    throw new FS.ErrnoError(60);
   }
   try {
    for (var i = 0; i < length; i++) {
     stream.tty.ops.put_char(stream.tty, buffer[offset + i]);
    }
   } catch (e) {
    throw new FS.ErrnoError(29);
   }
   if (length) {
    stream.node.timestamp = Date.now();
   }
   return i;
  }
 },
 default_tty_ops: {
  get_char: function(tty) {
   if (!tty.input.length) {
    var result = null;
    if (ENVIRONMENT_IS_NODE) {
     var BUFSIZE = 256;
     var buf = Buffer.alloc(BUFSIZE);
     var bytesRead = 0;
     try {
      bytesRead = nodeFS.readSync(process.stdin.fd, buf, 0, BUFSIZE, null);
     } catch (e) {
      if (e.toString().includes("EOF")) bytesRead = 0; else throw e;
     }
     if (bytesRead > 0) {
      result = buf.slice(0, bytesRead).toString("utf-8");
     } else {
      result = null;
     }
    } else if (typeof window != "undefined" && typeof window.prompt == "function") {
     result = window.prompt("Input: ");
     if (result !== null) {
      result += "\n";
     }
    } else if (typeof readline == "function") {
     result = readline();
     if (result !== null) {
      result += "\n";
     }
    }
    if (!result) {
     return null;
    }
    tty.input = intArrayFromString(result, true);
   }
   return tty.input.shift();
  },
  put_char: function(tty, val) {
   if (val === null || val === 10) {
    out(UTF8ArrayToString(tty.output, 0));
    tty.output = [];
   } else {
    if (val != 0) tty.output.push(val);
   }
  },
  flush: function(tty) {
   if (tty.output && tty.output.length > 0) {
    out(UTF8ArrayToString(tty.output, 0));
    tty.output = [];
   }
  }
 },
 default_tty1_ops: {
  put_char: function(tty, val) {
   if (val === null || val === 10) {
    err(UTF8ArrayToString(tty.output, 0));
    tty.output = [];
   } else {
    if (val != 0) tty.output.push(val);
   }
  },
  flush: function(tty) {
   if (tty.output && tty.output.length > 0) {
    err(UTF8ArrayToString(tty.output, 0));
    tty.output = [];
   }
  }
 }
};

function zeroMemory(address, size) {
 HEAPU8.fill(0, address, address + size);
}

function alignMemory(size, alignment) {
 assert(alignment, "alignment argument is required");
 return Math.ceil(size / alignment) * alignment;
}

function mmapAlloc(size) {
 size = alignMemory(size, 65536);
 var ptr = _memalign(65536, size);
 if (!ptr) return 0;
 zeroMemory(ptr, size);
 return ptr;
}

var MEMFS = {
 ops_table: null,
 mount: function(mount) {
  return MEMFS.createNode(null, "/", 16384 | 511, 0);
 },
 createNode: function(parent, name, mode, dev) {
  if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
   throw new FS.ErrnoError(63);
  }
  if (!MEMFS.ops_table) {
   MEMFS.ops_table = {
    dir: {
     node: {
      getattr: MEMFS.node_ops.getattr,
      setattr: MEMFS.node_ops.setattr,
      lookup: MEMFS.node_ops.lookup,
      mknod: MEMFS.node_ops.mknod,
      rename: MEMFS.node_ops.rename,
      unlink: MEMFS.node_ops.unlink,
      rmdir: MEMFS.node_ops.rmdir,
      readdir: MEMFS.node_ops.readdir,
      symlink: MEMFS.node_ops.symlink
     },
     stream: {
      llseek: MEMFS.stream_ops.llseek
     }
    },
    file: {
     node: {
      getattr: MEMFS.node_ops.getattr,
      setattr: MEMFS.node_ops.setattr
     },
     stream: {
      llseek: MEMFS.stream_ops.llseek,
      read: MEMFS.stream_ops.read,
      write: MEMFS.stream_ops.write,
      allocate: MEMFS.stream_ops.allocate,
      mmap: MEMFS.stream_ops.mmap,
      msync: MEMFS.stream_ops.msync
     }
    },
    link: {
     node: {
      getattr: MEMFS.node_ops.getattr,
      setattr: MEMFS.node_ops.setattr,
      readlink: MEMFS.node_ops.readlink
     },
     stream: {}
    },
    chrdev: {
     node: {
      getattr: MEMFS.node_ops.getattr,
      setattr: MEMFS.node_ops.setattr
     },
     stream: FS.chrdev_stream_ops
    }
   };
  }
  var node = FS.createNode(parent, name, mode, dev);
  if (FS.isDir(node.mode)) {
   node.node_ops = MEMFS.ops_table.dir.node;
   node.stream_ops = MEMFS.ops_table.dir.stream;
   node.contents = {};
  } else if (FS.isFile(node.mode)) {
   node.node_ops = MEMFS.ops_table.file.node;
   node.stream_ops = MEMFS.ops_table.file.stream;
   node.usedBytes = 0;
   node.contents = null;
  } else if (FS.isLink(node.mode)) {
   node.node_ops = MEMFS.ops_table.link.node;
   node.stream_ops = MEMFS.ops_table.link.stream;
  } else if (FS.isChrdev(node.mode)) {
   node.node_ops = MEMFS.ops_table.chrdev.node;
   node.stream_ops = MEMFS.ops_table.chrdev.stream;
  }
  node.timestamp = Date.now();
  if (parent) {
   parent.contents[name] = node;
   parent.timestamp = node.timestamp;
  }
  return node;
 },
 getFileDataAsTypedArray: function(node) {
  if (!node.contents) return new Uint8Array(0);
  if (node.contents.subarray) return node.contents.subarray(0, node.usedBytes);
  return new Uint8Array(node.contents);
 },
 expandFileStorage: function(node, newCapacity) {
  var prevCapacity = node.contents ? node.contents.length : 0;
  if (prevCapacity >= newCapacity) return;
  var CAPACITY_DOUBLING_MAX = 1024 * 1024;
  newCapacity = Math.max(newCapacity, prevCapacity * (prevCapacity < CAPACITY_DOUBLING_MAX ? 2 : 1.125) >>> 0);
  if (prevCapacity != 0) newCapacity = Math.max(newCapacity, 256);
  var oldContents = node.contents;
  node.contents = new Uint8Array(newCapacity);
  if (node.usedBytes > 0) node.contents.set(oldContents.subarray(0, node.usedBytes), 0);
 },
 resizeFileStorage: function(node, newSize) {
  if (node.usedBytes == newSize) return;
  if (newSize == 0) {
   node.contents = null;
   node.usedBytes = 0;
  } else {
   var oldContents = node.contents;
   node.contents = new Uint8Array(newSize);
   if (oldContents) {
    node.contents.set(oldContents.subarray(0, Math.min(newSize, node.usedBytes)));
   }
   node.usedBytes = newSize;
  }
 },
 node_ops: {
  getattr: function(node) {
   var attr = {};
   attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
   attr.ino = node.id;
   attr.mode = node.mode;
   attr.nlink = 1;
   attr.uid = 0;
   attr.gid = 0;
   attr.rdev = node.rdev;
   if (FS.isDir(node.mode)) {
    attr.size = 4096;
   } else if (FS.isFile(node.mode)) {
    attr.size = node.usedBytes;
   } else if (FS.isLink(node.mode)) {
    attr.size = node.link.length;
   } else {
    attr.size = 0;
   }
   attr.atime = new Date(node.timestamp);
   attr.mtime = new Date(node.timestamp);
   attr.ctime = new Date(node.timestamp);
   attr.blksize = 4096;
   attr.blocks = Math.ceil(attr.size / attr.blksize);
   return attr;
  },
  setattr: function(node, attr) {
   if (attr.mode !== undefined) {
    node.mode = attr.mode;
   }
   if (attr.timestamp !== undefined) {
    node.timestamp = attr.timestamp;
   }
   if (attr.size !== undefined) {
    MEMFS.resizeFileStorage(node, attr.size);
   }
  },
  lookup: function(parent, name) {
   throw FS.genericErrors[44];
  },
  mknod: function(parent, name, mode, dev) {
   return MEMFS.createNode(parent, name, mode, dev);
  },
  rename: function(old_node, new_dir, new_name) {
   if (FS.isDir(old_node.mode)) {
    var new_node;
    try {
     new_node = FS.lookupNode(new_dir, new_name);
    } catch (e) {}
    if (new_node) {
     for (var i in new_node.contents) {
      throw new FS.ErrnoError(55);
     }
    }
   }
   delete old_node.parent.contents[old_node.name];
   old_node.parent.timestamp = Date.now();
   old_node.name = new_name;
   new_dir.contents[new_name] = old_node;
   new_dir.timestamp = old_node.parent.timestamp;
   old_node.parent = new_dir;
  },
  unlink: function(parent, name) {
   delete parent.contents[name];
   parent.timestamp = Date.now();
  },
  rmdir: function(parent, name) {
   var node = FS.lookupNode(parent, name);
   for (var i in node.contents) {
    throw new FS.ErrnoError(55);
   }
   delete parent.contents[name];
   parent.timestamp = Date.now();
  },
  readdir: function(node) {
   var entries = [ ".", ".." ];
   for (var key in node.contents) {
    if (!node.contents.hasOwnProperty(key)) {
     continue;
    }
    entries.push(key);
   }
   return entries;
  },
  symlink: function(parent, newname, oldpath) {
   var node = MEMFS.createNode(parent, newname, 511 | 40960, 0);
   node.link = oldpath;
   return node;
  },
  readlink: function(node) {
   if (!FS.isLink(node.mode)) {
    throw new FS.ErrnoError(28);
   }
   return node.link;
  }
 },
 stream_ops: {
  read: function(stream, buffer, offset, length, position) {
   var contents = stream.node.contents;
   if (position >= stream.node.usedBytes) return 0;
   var size = Math.min(stream.node.usedBytes - position, length);
   assert(size >= 0);
   if (size > 8 && contents.subarray) {
    buffer.set(contents.subarray(position, position + size), offset);
   } else {
    for (var i = 0; i < size; i++) buffer[offset + i] = contents[position + i];
   }
   return size;
  },
  write: function(stream, buffer, offset, length, position, canOwn) {
   assert(!(buffer instanceof ArrayBuffer));
   if (buffer.buffer === HEAP8.buffer) {
    canOwn = false;
   }
   if (!length) return 0;
   var node = stream.node;
   node.timestamp = Date.now();
   if (buffer.subarray && (!node.contents || node.contents.subarray)) {
    if (canOwn) {
     assert(position === 0, "canOwn must imply no weird position inside the file");
     node.contents = buffer.subarray(offset, offset + length);
     node.usedBytes = length;
     return length;
    } else if (node.usedBytes === 0 && position === 0) {
     node.contents = buffer.slice(offset, offset + length);
     node.usedBytes = length;
     return length;
    } else if (position + length <= node.usedBytes) {
     node.contents.set(buffer.subarray(offset, offset + length), position);
     return length;
    }
   }
   MEMFS.expandFileStorage(node, position + length);
   if (node.contents.subarray && buffer.subarray) {
    node.contents.set(buffer.subarray(offset, offset + length), position);
   } else {
    for (var i = 0; i < length; i++) {
     node.contents[position + i] = buffer[offset + i];
    }
   }
   node.usedBytes = Math.max(node.usedBytes, position + length);
   return length;
  },
  llseek: function(stream, offset, whence) {
   var position = offset;
   if (whence === 1) {
    position += stream.position;
   } else if (whence === 2) {
    if (FS.isFile(stream.node.mode)) {
     position += stream.node.usedBytes;
    }
   }
   if (position < 0) {
    throw new FS.ErrnoError(28);
   }
   return position;
  },
  allocate: function(stream, offset, length) {
   MEMFS.expandFileStorage(stream.node, offset + length);
   stream.node.usedBytes = Math.max(stream.node.usedBytes, offset + length);
  },
  mmap: function(stream, address, length, position, prot, flags) {
   if (address !== 0) {
    throw new FS.ErrnoError(28);
   }
   if (!FS.isFile(stream.node.mode)) {
    throw new FS.ErrnoError(43);
   }
   var ptr;
   var allocated;
   var contents = stream.node.contents;
   if (!(flags & 2) && contents.buffer === buffer) {
    allocated = false;
    ptr = contents.byteOffset;
   } else {
    if (position > 0 || position + length < contents.length) {
     if (contents.subarray) {
      contents = contents.subarray(position, position + length);
     } else {
      contents = Array.prototype.slice.call(contents, position, position + length);
     }
    }
    allocated = true;
    ptr = mmapAlloc(length);
    if (!ptr) {
     throw new FS.ErrnoError(48);
    }
    HEAP8.set(contents, ptr);
   }
   return {
    ptr: ptr,
    allocated: allocated
   };
  },
  msync: function(stream, buffer, offset, length, mmapFlags) {
   if (!FS.isFile(stream.node.mode)) {
    throw new FS.ErrnoError(43);
   }
   if (mmapFlags & 2) {
    return 0;
   }
   var bytesWritten = MEMFS.stream_ops.write(stream, buffer, 0, length, offset, false);
   return 0;
  }
 }
};

function asyncLoad(url, onload, onerror, noRunDep) {
 var dep = !noRunDep ? getUniqueRunDependency("al " + url) : "";
 readAsync(url, function(arrayBuffer) {
  assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
  onload(new Uint8Array(arrayBuffer));
  if (dep) removeRunDependency(dep);
 }, function(event) {
  if (onerror) {
   onerror();
  } else {
   throw 'Loading data file "' + url + '" failed.';
  }
 });
 if (dep) addRunDependency(dep);
}

var ERRNO_MESSAGES = {
 0: "Success",
 1: "Arg list too long",
 2: "Permission denied",
 3: "Address already in use",
 4: "Address not available",
 5: "Address family not supported by protocol family",
 6: "No more processes",
 7: "Socket already connected",
 8: "Bad file number",
 9: "Trying to read unreadable message",
 10: "Mount device busy",
 11: "Operation canceled",
 12: "No children",
 13: "Connection aborted",
 14: "Connection refused",
 15: "Connection reset by peer",
 16: "File locking deadlock error",
 17: "Destination address required",
 18: "Math arg out of domain of func",
 19: "Quota exceeded",
 20: "File exists",
 21: "Bad address",
 22: "File too large",
 23: "Host is unreachable",
 24: "Identifier removed",
 25: "Illegal byte sequence",
 26: "Connection already in progress",
 27: "Interrupted system call",
 28: "Invalid argument",
 29: "I/O error",
 30: "Socket is already connected",
 31: "Is a directory",
 32: "Too many symbolic links",
 33: "Too many open files",
 34: "Too many links",
 35: "Message too long",
 36: "Multihop attempted",
 37: "File or path name too long",
 38: "Network interface is not configured",
 39: "Connection reset by network",
 40: "Network is unreachable",
 41: "Too many open files in system",
 42: "No buffer space available",
 43: "No such device",
 44: "No such file or directory",
 45: "Exec format error",
 46: "No record locks available",
 47: "The link has been severed",
 48: "Not enough core",
 49: "No message of desired type",
 50: "Protocol not available",
 51: "No space left on device",
 52: "Function not implemented",
 53: "Socket is not connected",
 54: "Not a directory",
 55: "Directory not empty",
 56: "State not recoverable",
 57: "Socket operation on non-socket",
 59: "Not a typewriter",
 60: "No such device or address",
 61: "Value too large for defined data type",
 62: "Previous owner died",
 63: "Not super-user",
 64: "Broken pipe",
 65: "Protocol error",
 66: "Unknown protocol",
 67: "Protocol wrong type for socket",
 68: "Math result not representable",
 69: "Read only file system",
 70: "Illegal seek",
 71: "No such process",
 72: "Stale file handle",
 73: "Connection timed out",
 74: "Text file busy",
 75: "Cross-device link",
 100: "Device not a stream",
 101: "Bad font file fmt",
 102: "Invalid slot",
 103: "Invalid request code",
 104: "No anode",
 105: "Block device required",
 106: "Channel number out of range",
 107: "Level 3 halted",
 108: "Level 3 reset",
 109: "Link number out of range",
 110: "Protocol driver not attached",
 111: "No CSI structure available",
 112: "Level 2 halted",
 113: "Invalid exchange",
 114: "Invalid request descriptor",
 115: "Exchange full",
 116: "No data (for no delay io)",
 117: "Timer expired",
 118: "Out of streams resources",
 119: "Machine is not on the network",
 120: "Package not installed",
 121: "The object is remote",
 122: "Advertise error",
 123: "Srmount error",
 124: "Communication error on send",
 125: "Cross mount point (not really error)",
 126: "Given log. name not unique",
 127: "f.d. invalid for this operation",
 128: "Remote address changed",
 129: "Can   access a needed shared lib",
 130: "Accessing a corrupted shared lib",
 131: ".lib section in a.out corrupted",
 132: "Attempting to link in too many libs",
 133: "Attempting to exec a shared library",
 135: "Streams pipe error",
 136: "Too many users",
 137: "Socket type not supported",
 138: "Not supported",
 139: "Protocol family not supported",
 140: "Can't send after socket shutdown",
 141: "Too many references",
 142: "Host is down",
 148: "No medium (in tape drive)",
 156: "Level 2 not synchronized"
};

var ERRNO_CODES = {};

var FS = {
 root: null,
 mounts: [],
 devices: {},
 streams: [],
 nextInode: 1,
 nameTable: null,
 currentPath: "/",
 initialized: false,
 ignorePermissions: true,
 ErrnoError: null,
 genericErrors: {},
 filesystems: null,
 syncFSRequests: 0,
 lookupPath: function(path, opts) {
  path = PATH_FS.resolve(FS.cwd(), path);
  opts = opts || {};
  if (!path) return {
   path: "",
   node: null
  };
  var defaults = {
   follow_mount: true,
   recurse_count: 0
  };
  for (var key in defaults) {
   if (opts[key] === undefined) {
    opts[key] = defaults[key];
   }
  }
  if (opts.recurse_count > 8) {
   throw new FS.ErrnoError(32);
  }
  var parts = PATH.normalizeArray(path.split("/").filter(function(p) {
   return !!p;
  }), false);
  var current = FS.root;
  var current_path = "/";
  for (var i = 0; i < parts.length; i++) {
   var islast = i === parts.length - 1;
   if (islast && opts.parent) {
    break;
   }
   current = FS.lookupNode(current, parts[i]);
   current_path = PATH.join2(current_path, parts[i]);
   if (FS.isMountpoint(current)) {
    if (!islast || islast && opts.follow_mount) {
     current = current.mounted.root;
    }
   }
   if (!islast || opts.follow) {
    var count = 0;
    while (FS.isLink(current.mode)) {
     var link = FS.readlink(current_path);
     current_path = PATH_FS.resolve(PATH.dirname(current_path), link);
     var lookup = FS.lookupPath(current_path, {
      recurse_count: opts.recurse_count
     });
     current = lookup.node;
     if (count++ > 40) {
      throw new FS.ErrnoError(32);
     }
    }
   }
  }
  return {
   path: current_path,
   node: current
  };
 },
 getPath: function(node) {
  var path;
  while (true) {
   if (FS.isRoot(node)) {
    var mount = node.mount.mountpoint;
    if (!path) return mount;
    return mount[mount.length - 1] !== "/" ? mount + "/" + path : mount + path;
   }
   path = path ? node.name + "/" + path : node.name;
   node = node.parent;
  }
 },
 hashName: function(parentid, name) {
  var hash = 0;
  name = name.toLowerCase();
  for (var i = 0; i < name.length; i++) {
   hash = (hash << 5) - hash + name.charCodeAt(i) | 0;
  }
  return (parentid + hash >>> 0) % FS.nameTable.length;
 },
 hashAddNode: function(node) {
  var hash = FS.hashName(node.parent.id, node.name);
  node.name_next = FS.nameTable[hash];
  FS.nameTable[hash] = node;
 },
 hashRemoveNode: function(node) {
  var hash = FS.hashName(node.parent.id, node.name);
  if (FS.nameTable[hash] === node) {
   FS.nameTable[hash] = node.name_next;
  } else {
   var current = FS.nameTable[hash];
   while (current) {
    if (current.name_next === node) {
     current.name_next = node.name_next;
     break;
    }
    current = current.name_next;
   }
  }
 },
 lookupNode: function(parent, name) {
  var errCode = FS.mayLookup(parent);
  if (errCode) {
   throw new FS.ErrnoError(errCode, parent);
  }
  var hash = FS.hashName(parent.id, name);
  name = name.toLowerCase();
  for (var node = FS.nameTable[hash]; node; node = node.name_next) {
   var nodeName = node.name;
   nodeName = nodeName.toLowerCase();
   if (node.parent.id === parent.id && nodeName === name) {
    return node;
   }
  }
  return FS.lookup(parent, name);
 },
 createNode: function(parent, name, mode, rdev) {
  assert(typeof parent === "object");
  var node = new FS.FSNode(parent, name, mode, rdev);
  FS.hashAddNode(node);
  return node;
 },
 destroyNode: function(node) {
  FS.hashRemoveNode(node);
 },
 isRoot: function(node) {
  return node === node.parent;
 },
 isMountpoint: function(node) {
  return !!node.mounted;
 },
 isFile: function(mode) {
  return (mode & 61440) === 32768;
 },
 isDir: function(mode) {
  return (mode & 61440) === 16384;
 },
 isLink: function(mode) {
  return (mode & 61440) === 40960;
 },
 isChrdev: function(mode) {
  return (mode & 61440) === 8192;
 },
 isBlkdev: function(mode) {
  return (mode & 61440) === 24576;
 },
 isFIFO: function(mode) {
  return (mode & 61440) === 4096;
 },
 isSocket: function(mode) {
  return (mode & 49152) === 49152;
 },
 flagModes: {
  "r": 0,
  "r+": 2,
  "w": 577,
  "w+": 578,
  "a": 1089,
  "a+": 1090
 },
 modeStringToFlags: function(str) {
  var flags = FS.flagModes[str];
  if (typeof flags === "undefined") {
   throw new Error("Unknown file open mode: " + str);
  }
  return flags;
 },
 flagsToPermissionString: function(flag) {
  var perms = [ "r", "w", "rw" ][flag & 3];
  if (flag & 512) {
   perms += "w";
  }
  return perms;
 },
 nodePermissions: function(node, perms) {
  if (FS.ignorePermissions) {
   return 0;
  }
  if (perms.includes("r") && !(node.mode & 292)) {
   return 2;
  } else if (perms.includes("w") && !(node.mode & 146)) {
   return 2;
  } else if (perms.includes("x") && !(node.mode & 73)) {
   return 2;
  }
  return 0;
 },
 mayLookup: function(dir) {
  var errCode = FS.nodePermissions(dir, "x");
  if (errCode) return errCode;
  if (!dir.node_ops.lookup) return 2;
  return 0;
 },
 mayCreate: function(dir, name) {
  try {
   var node = FS.lookupNode(dir, name);
   return 20;
  } catch (e) {}
  return FS.nodePermissions(dir, "wx");
 },
 mayDelete: function(dir, name, isdir) {
  var node;
  try {
   node = FS.lookupNode(dir, name);
  } catch (e) {
   return e.errno;
  }
  var errCode = FS.nodePermissions(dir, "wx");
  if (errCode) {
   return errCode;
  }
  if (isdir) {
   if (!FS.isDir(node.mode)) {
    return 54;
   }
   if (FS.isRoot(node) || FS.getPath(node) === FS.cwd()) {
    return 10;
   }
  } else {
   if (FS.isDir(node.mode)) {
    return 31;
   }
  }
  return 0;
 },
 mayOpen: function(node, flags) {
  if (!node) {
   return 44;
  }
  if (FS.isLink(node.mode)) {
   return 32;
  } else if (FS.isDir(node.mode)) {
   if (FS.flagsToPermissionString(flags) !== "r" || flags & 512) {
    return 31;
   }
  }
  return FS.nodePermissions(node, FS.flagsToPermissionString(flags));
 },
 MAX_OPEN_FDS: 4096,
 nextfd: function(fd_start, fd_end) {
  fd_start = fd_start || 0;
  fd_end = fd_end || FS.MAX_OPEN_FDS;
  for (var fd = fd_start; fd <= fd_end; fd++) {
   if (!FS.streams[fd]) {
    return fd;
   }
  }
  throw new FS.ErrnoError(33);
 },
 getStream: function(fd) {
  return FS.streams[fd];
 },
 createStream: function(stream, fd_start, fd_end) {
  if (!FS.FSStream) {
   FS.FSStream = function() {};
   FS.FSStream.prototype = {
    object: {
     get: function() {
      return this.node;
     },
     set: function(val) {
      this.node = val;
     }
    },
    isRead: {
     get: function() {
      return (this.flags & 2097155) !== 1;
     }
    },
    isWrite: {
     get: function() {
      return (this.flags & 2097155) !== 0;
     }
    },
    isAppend: {
     get: function() {
      return this.flags & 1024;
     }
    }
   };
  }
  var newStream = new FS.FSStream();
  for (var p in stream) {
   newStream[p] = stream[p];
  }
  stream = newStream;
  var fd = FS.nextfd(fd_start, fd_end);
  stream.fd = fd;
  FS.streams[fd] = stream;
  return stream;
 },
 closeStream: function(fd) {
  FS.streams[fd] = null;
 },
 chrdev_stream_ops: {
  open: function(stream) {
   var device = FS.getDevice(stream.node.rdev);
   stream.stream_ops = device.stream_ops;
   if (stream.stream_ops.open) {
    stream.stream_ops.open(stream);
   }
  },
  llseek: function() {
   throw new FS.ErrnoError(70);
  }
 },
 major: function(dev) {
  return dev >> 8;
 },
 minor: function(dev) {
  return dev & 255;
 },
 makedev: function(ma, mi) {
  return ma << 8 | mi;
 },
 registerDevice: function(dev, ops) {
  FS.devices[dev] = {
   stream_ops: ops
  };
 },
 getDevice: function(dev) {
  return FS.devices[dev];
 },
 getMounts: function(mount) {
  var mounts = [];
  var check = [ mount ];
  while (check.length) {
   var m = check.pop();
   mounts.push(m);
   check.push.apply(check, m.mounts);
  }
  return mounts;
 },
 syncfs: function(populate, callback) {
  if (typeof populate === "function") {
   callback = populate;
   populate = false;
  }
  FS.syncFSRequests++;
  if (FS.syncFSRequests > 1) {
   err("warning: " + FS.syncFSRequests + " FS.syncfs operations in flight at once, probably just doing extra work");
  }
  var mounts = FS.getMounts(FS.root.mount);
  var completed = 0;
  function doCallback(errCode) {
   assert(FS.syncFSRequests > 0);
   FS.syncFSRequests--;
   return callback(errCode);
  }
  function done(errCode) {
   if (errCode) {
    if (!done.errored) {
     done.errored = true;
     return doCallback(errCode);
    }
    return;
   }
   if (++completed >= mounts.length) {
    doCallback(null);
   }
  }
  mounts.forEach(function(mount) {
   if (!mount.type.syncfs) {
    return done(null);
   }
   mount.type.syncfs(mount, populate, done);
  });
 },
 mount: function(type, opts, mountpoint) {
  if (typeof type === "string") {
   throw type;
  }
  var root = mountpoint === "/";
  var pseudo = !mountpoint;
  var node;
  if (root && FS.root) {
   throw new FS.ErrnoError(10);
  } else if (!root && !pseudo) {
   var lookup = FS.lookupPath(mountpoint, {
    follow_mount: false
   });
   mountpoint = lookup.path;
   node = lookup.node;
   if (FS.isMountpoint(node)) {
    throw new FS.ErrnoError(10);
   }
   if (!FS.isDir(node.mode)) {
    throw new FS.ErrnoError(54);
   }
  }
  var mount = {
   type: type,
   opts: opts,
   mountpoint: mountpoint,
   mounts: []
  };
  var mountRoot = type.mount(mount);
  mountRoot.mount = mount;
  mount.root = mountRoot;
  if (root) {
   FS.root = mountRoot;
  } else if (node) {
   node.mounted = mount;
   if (node.mount) {
    node.mount.mounts.push(mount);
   }
  }
  return mountRoot;
 },
 unmount: function(mountpoint) {
  var lookup = FS.lookupPath(mountpoint, {
   follow_mount: false
  });
  if (!FS.isMountpoint(lookup.node)) {
   throw new FS.ErrnoError(28);
  }
  var node = lookup.node;
  var mount = node.mounted;
  var mounts = FS.getMounts(mount);
  Object.keys(FS.nameTable).forEach(function(hash) {
   var current = FS.nameTable[hash];
   while (current) {
    var next = current.name_next;
    if (mounts.includes(current.mount)) {
     FS.destroyNode(current);
    }
    current = next;
   }
  });
  node.mounted = null;
  var idx = node.mount.mounts.indexOf(mount);
  assert(idx !== -1);
  node.mount.mounts.splice(idx, 1);
 },
 lookup: function(parent, name) {
  return parent.node_ops.lookup(parent, name);
 },
 mknod: function(path, mode, dev) {
  var lookup = FS.lookupPath(path, {
   parent: true
  });
  var parent = lookup.node;
  var name = PATH.basename(path);
  if (!name || name === "." || name === "..") {
   throw new FS.ErrnoError(28);
  }
  var errCode = FS.mayCreate(parent, name);
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  if (!parent.node_ops.mknod) {
   throw new FS.ErrnoError(63);
  }
  return parent.node_ops.mknod(parent, name, mode, dev);
 },
 create: function(path, mode) {
  mode = mode !== undefined ? mode : 438;
  mode &= 4095;
  mode |= 32768;
  return FS.mknod(path, mode, 0);
 },
 mkdir: function(path, mode) {
  mode = mode !== undefined ? mode : 511;
  mode &= 511 | 512;
  mode |= 16384;
  return FS.mknod(path, mode, 0);
 },
 mkdirTree: function(path, mode) {
  var dirs = path.split("/");
  var d = "";
  for (var i = 0; i < dirs.length; ++i) {
   if (!dirs[i]) continue;
   d += "/" + dirs[i];
   try {
    FS.mkdir(d, mode);
   } catch (e) {
    if (e.errno != 20) throw e;
   }
  }
 },
 mkdev: function(path, mode, dev) {
  if (typeof dev === "undefined") {
   dev = mode;
   mode = 438;
  }
  mode |= 8192;
  return FS.mknod(path, mode, dev);
 },
 symlink: function(oldpath, newpath) {
  if (!PATH_FS.resolve(oldpath)) {
   throw new FS.ErrnoError(44);
  }
  var lookup = FS.lookupPath(newpath, {
   parent: true
  });
  var parent = lookup.node;
  if (!parent) {
   throw new FS.ErrnoError(44);
  }
  var newname = PATH.basename(newpath);
  var errCode = FS.mayCreate(parent, newname);
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  if (!parent.node_ops.symlink) {
   throw new FS.ErrnoError(63);
  }
  return parent.node_ops.symlink(parent, newname, oldpath);
 },
 rename: function(old_path, new_path) {
  var old_dirname = PATH.dirname(old_path);
  var new_dirname = PATH.dirname(new_path);
  var old_name = PATH.basename(old_path);
  var new_name = PATH.basename(new_path);
  var lookup, old_dir, new_dir;
  lookup = FS.lookupPath(old_path, {
   parent: true
  });
  old_dir = lookup.node;
  lookup = FS.lookupPath(new_path, {
   parent: true
  });
  new_dir = lookup.node;
  if (!old_dir || !new_dir) throw new FS.ErrnoError(44);
  if (old_dir.mount !== new_dir.mount) {
   throw new FS.ErrnoError(75);
  }
  var old_node = FS.lookupNode(old_dir, old_name);
  var relative = PATH_FS.relative(old_path, new_dirname);
  if (relative.charAt(0) !== ".") {
   throw new FS.ErrnoError(28);
  }
  relative = PATH_FS.relative(new_path, old_dirname);
  if (relative.charAt(0) !== ".") {
   throw new FS.ErrnoError(55);
  }
  var new_node;
  try {
   new_node = FS.lookupNode(new_dir, new_name);
  } catch (e) {}
  if (old_node === new_node) {
   return;
  }
  var isdir = FS.isDir(old_node.mode);
  var errCode = FS.mayDelete(old_dir, old_name, isdir);
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  errCode = new_node ? FS.mayDelete(new_dir, new_name, isdir) : FS.mayCreate(new_dir, new_name);
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  if (!old_dir.node_ops.rename) {
   throw new FS.ErrnoError(63);
  }
  if (FS.isMountpoint(old_node) || new_node && FS.isMountpoint(new_node)) {
   throw new FS.ErrnoError(10);
  }
  if (new_dir !== old_dir) {
   errCode = FS.nodePermissions(old_dir, "w");
   if (errCode) {
    throw new FS.ErrnoError(errCode);
   }
  }
  FS.hashRemoveNode(old_node);
  try {
   old_dir.node_ops.rename(old_node, new_dir, new_name);
  } catch (e) {
   throw e;
  } finally {
   FS.hashAddNode(old_node);
  }
 },
 rmdir: function(path) {
  var lookup = FS.lookupPath(path, {
   parent: true
  });
  var parent = lookup.node;
  var name = PATH.basename(path);
  var node = FS.lookupNode(parent, name);
  var errCode = FS.mayDelete(parent, name, true);
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  if (!parent.node_ops.rmdir) {
   throw new FS.ErrnoError(63);
  }
  if (FS.isMountpoint(node)) {
   throw new FS.ErrnoError(10);
  }
  parent.node_ops.rmdir(parent, name);
  FS.destroyNode(node);
 },
 readdir: function(path) {
  var lookup = FS.lookupPath(path, {
   follow: true
  });
  var node = lookup.node;
  if (!node.node_ops.readdir) {
   throw new FS.ErrnoError(54);
  }
  return node.node_ops.readdir(node);
 },
 unlink: function(path) {
  var lookup = FS.lookupPath(path, {
   parent: true
  });
  var parent = lookup.node;
  var name = PATH.basename(path);
  var node = FS.lookupNode(parent, name);
  var errCode = FS.mayDelete(parent, name, false);
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  if (!parent.node_ops.unlink) {
   throw new FS.ErrnoError(63);
  }
  if (FS.isMountpoint(node)) {
   throw new FS.ErrnoError(10);
  }
  parent.node_ops.unlink(parent, name);
  FS.destroyNode(node);
 },
 readlink: function(path) {
  var lookup = FS.lookupPath(path);
  var link = lookup.node;
  if (!link) {
   throw new FS.ErrnoError(44);
  }
  if (!link.node_ops.readlink) {
   throw new FS.ErrnoError(28);
  }
  return PATH_FS.resolve(FS.getPath(link.parent), link.node_ops.readlink(link));
 },
 stat: function(path, dontFollow) {
  var lookup = FS.lookupPath(path, {
   follow: !dontFollow
  });
  var node = lookup.node;
  if (!node) {
   throw new FS.ErrnoError(44);
  }
  if (!node.node_ops.getattr) {
   throw new FS.ErrnoError(63);
  }
  return node.node_ops.getattr(node);
 },
 lstat: function(path) {
  return FS.stat(path, true);
 },
 chmod: function(path, mode, dontFollow) {
  var node;
  if (typeof path === "string") {
   var lookup = FS.lookupPath(path, {
    follow: !dontFollow
   });
   node = lookup.node;
  } else {
   node = path;
  }
  if (!node.node_ops.setattr) {
   throw new FS.ErrnoError(63);
  }
  node.node_ops.setattr(node, {
   mode: mode & 4095 | node.mode & ~4095,
   timestamp: Date.now()
  });
 },
 lchmod: function(path, mode) {
  FS.chmod(path, mode, true);
 },
 fchmod: function(fd, mode) {
  var stream = FS.getStream(fd);
  if (!stream) {
   throw new FS.ErrnoError(8);
  }
  FS.chmod(stream.node, mode);
 },
 chown: function(path, uid, gid, dontFollow) {
  var node;
  if (typeof path === "string") {
   var lookup = FS.lookupPath(path, {
    follow: !dontFollow
   });
   node = lookup.node;
  } else {
   node = path;
  }
  if (!node.node_ops.setattr) {
   throw new FS.ErrnoError(63);
  }
  node.node_ops.setattr(node, {
   timestamp: Date.now()
  });
 },
 lchown: function(path, uid, gid) {
  FS.chown(path, uid, gid, true);
 },
 fchown: function(fd, uid, gid) {
  var stream = FS.getStream(fd);
  if (!stream) {
   throw new FS.ErrnoError(8);
  }
  FS.chown(stream.node, uid, gid);
 },
 truncate: function(path, len) {
  if (len < 0) {
   throw new FS.ErrnoError(28);
  }
  var node;
  if (typeof path === "string") {
   var lookup = FS.lookupPath(path, {
    follow: true
   });
   node = lookup.node;
  } else {
   node = path;
  }
  if (!node.node_ops.setattr) {
   throw new FS.ErrnoError(63);
  }
  if (FS.isDir(node.mode)) {
   throw new FS.ErrnoError(31);
  }
  if (!FS.isFile(node.mode)) {
   throw new FS.ErrnoError(28);
  }
  var errCode = FS.nodePermissions(node, "w");
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  node.node_ops.setattr(node, {
   size: len,
   timestamp: Date.now()
  });
 },
 ftruncate: function(fd, len) {
  var stream = FS.getStream(fd);
  if (!stream) {
   throw new FS.ErrnoError(8);
  }
  if ((stream.flags & 2097155) === 0) {
   throw new FS.ErrnoError(28);
  }
  FS.truncate(stream.node, len);
 },
 utime: function(path, atime, mtime) {
  var lookup = FS.lookupPath(path, {
   follow: true
  });
  var node = lookup.node;
  node.node_ops.setattr(node, {
   timestamp: Math.max(atime, mtime)
  });
 },
 open: function(path, flags, mode, fd_start, fd_end) {
  if (path === "") {
   throw new FS.ErrnoError(44);
  }
  flags = typeof flags === "string" ? FS.modeStringToFlags(flags) : flags;
  mode = typeof mode === "undefined" ? 438 : mode;
  if (flags & 64) {
   mode = mode & 4095 | 32768;
  } else {
   mode = 0;
  }
  var node;
  if (typeof path === "object") {
   node = path;
  } else {
   path = PATH.normalize(path);
   try {
    var lookup = FS.lookupPath(path, {
     follow: !(flags & 131072)
    });
    node = lookup.node;
   } catch (e) {}
  }
  var created = false;
  if (flags & 64) {
   if (node) {
    if (flags & 128) {
     throw new FS.ErrnoError(20);
    }
   } else {
    node = FS.mknod(path, mode, 0);
    created = true;
   }
  }
  if (!node) {
   throw new FS.ErrnoError(44);
  }
  if (FS.isChrdev(node.mode)) {
   flags &= ~512;
  }
  if (flags & 65536 && !FS.isDir(node.mode)) {
   throw new FS.ErrnoError(54);
  }
  if (!created) {
   var errCode = FS.mayOpen(node, flags);
   if (errCode) {
    throw new FS.ErrnoError(errCode);
   }
  }
  if (flags & 512) {
   FS.truncate(node, 0);
  }
  flags &= ~(128 | 512 | 131072);
  var stream = FS.createStream({
   node: node,
   path: FS.getPath(node),
   flags: flags,
   seekable: true,
   position: 0,
   stream_ops: node.stream_ops,
   ungotten: [],
   error: false
  }, fd_start, fd_end);
  if (stream.stream_ops.open) {
   stream.stream_ops.open(stream);
  }
  if (Module["logReadFiles"] && !(flags & 1)) {
   if (!FS.readFiles) FS.readFiles = {};
   if (!(path in FS.readFiles)) {
    FS.readFiles[path] = 1;
   }
  }
  return stream;
 },
 close: function(stream) {
  if (FS.isClosed(stream)) {
   throw new FS.ErrnoError(8);
  }
  if (stream.getdents) stream.getdents = null;
  try {
   if (stream.stream_ops.close) {
    stream.stream_ops.close(stream);
   }
  } catch (e) {
   throw e;
  } finally {
   FS.closeStream(stream.fd);
  }
  stream.fd = null;
 },
 isClosed: function(stream) {
  return stream.fd === null;
 },
 llseek: function(stream, offset, whence) {
  if (FS.isClosed(stream)) {
   throw new FS.ErrnoError(8);
  }
  if (!stream.seekable || !stream.stream_ops.llseek) {
   throw new FS.ErrnoError(70);
  }
  if (whence != 0 && whence != 1 && whence != 2) {
   throw new FS.ErrnoError(28);
  }
  stream.position = stream.stream_ops.llseek(stream, offset, whence);
  stream.ungotten = [];
  return stream.position;
 },
 read: function(stream, buffer, offset, length, position) {
  if (length < 0 || position < 0) {
   throw new FS.ErrnoError(28);
  }
  if (FS.isClosed(stream)) {
   throw new FS.ErrnoError(8);
  }
  if ((stream.flags & 2097155) === 1) {
   throw new FS.ErrnoError(8);
  }
  if (FS.isDir(stream.node.mode)) {
   throw new FS.ErrnoError(31);
  }
  if (!stream.stream_ops.read) {
   throw new FS.ErrnoError(28);
  }
  var seeking = typeof position !== "undefined";
  if (!seeking) {
   position = stream.position;
  } else if (!stream.seekable) {
   throw new FS.ErrnoError(70);
  }
  var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
  if (!seeking) stream.position += bytesRead;
  return bytesRead;
 },
 write: function(stream, buffer, offset, length, position, canOwn) {
  if (length < 0 || position < 0) {
   throw new FS.ErrnoError(28);
  }
  if (FS.isClosed(stream)) {
   throw new FS.ErrnoError(8);
  }
  if ((stream.flags & 2097155) === 0) {
   throw new FS.ErrnoError(8);
  }
  if (FS.isDir(stream.node.mode)) {
   throw new FS.ErrnoError(31);
  }
  if (!stream.stream_ops.write) {
   throw new FS.ErrnoError(28);
  }
  if (stream.seekable && stream.flags & 1024) {
   FS.llseek(stream, 0, 2);
  }
  var seeking = typeof position !== "undefined";
  if (!seeking) {
   position = stream.position;
  } else if (!stream.seekable) {
   throw new FS.ErrnoError(70);
  }
  var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
  if (!seeking) stream.position += bytesWritten;
  return bytesWritten;
 },
 allocate: function(stream, offset, length) {
  if (FS.isClosed(stream)) {
   throw new FS.ErrnoError(8);
  }
  if (offset < 0 || length <= 0) {
   throw new FS.ErrnoError(28);
  }
  if ((stream.flags & 2097155) === 0) {
   throw new FS.ErrnoError(8);
  }
  if (!FS.isFile(stream.node.mode) && !FS.isDir(stream.node.mode)) {
   throw new FS.ErrnoError(43);
  }
  if (!stream.stream_ops.allocate) {
   throw new FS.ErrnoError(138);
  }
  stream.stream_ops.allocate(stream, offset, length);
 },
 mmap: function(stream, address, length, position, prot, flags) {
  if ((prot & 2) !== 0 && (flags & 2) === 0 && (stream.flags & 2097155) !== 2) {
   throw new FS.ErrnoError(2);
  }
  if ((stream.flags & 2097155) === 1) {
   throw new FS.ErrnoError(2);
  }
  if (!stream.stream_ops.mmap) {
   throw new FS.ErrnoError(43);
  }
  return stream.stream_ops.mmap(stream, address, length, position, prot, flags);
 },
 msync: function(stream, buffer, offset, length, mmapFlags) {
  if (!stream || !stream.stream_ops.msync) {
   return 0;
  }
  return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
 },
 munmap: function(stream) {
  return 0;
 },
 ioctl: function(stream, cmd, arg) {
  if (!stream.stream_ops.ioctl) {
   throw new FS.ErrnoError(59);
  }
  return stream.stream_ops.ioctl(stream, cmd, arg);
 },
 readFile: function(path, opts) {
  opts = opts || {};
  opts.flags = opts.flags || 0;
  opts.encoding = opts.encoding || "binary";
  if (opts.encoding !== "utf8" && opts.encoding !== "binary") {
   throw new Error('Invalid encoding type "' + opts.encoding + '"');
  }
  var ret;
  var stream = FS.open(path, opts.flags);
  var stat = FS.stat(path);
  var length = stat.size;
  var buf = new Uint8Array(length);
  FS.read(stream, buf, 0, length, 0);
  if (opts.encoding === "utf8") {
   ret = UTF8ArrayToString(buf, 0);
  } else if (opts.encoding === "binary") {
   ret = buf;
  }
  FS.close(stream);
  return ret;
 },
 writeFile: function(path, data, opts) {
  opts = opts || {};
  opts.flags = opts.flags || 577;
  var stream = FS.open(path, opts.flags, opts.mode);
  if (typeof data === "string") {
   var buf = new Uint8Array(lengthBytesUTF8(data) + 1);
   var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
   FS.write(stream, buf, 0, actualNumBytes, undefined, opts.canOwn);
  } else if (ArrayBuffer.isView(data)) {
   FS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn);
  } else {
   throw new Error("Unsupported data type");
  }
  FS.close(stream);
 },
 cwd: function() {
  return FS.currentPath;
 },
 chdir: function(path) {
  var lookup = FS.lookupPath(path, {
   follow: true
  });
  if (lookup.node === null) {
   throw new FS.ErrnoError(44);
  }
  if (!FS.isDir(lookup.node.mode)) {
   throw new FS.ErrnoError(54);
  }
  var errCode = FS.nodePermissions(lookup.node, "x");
  if (errCode) {
   throw new FS.ErrnoError(errCode);
  }
  FS.currentPath = lookup.path;
 },
 createDefaultDirectories: function() {
  FS.mkdir("/tmp");
  FS.mkdir("/home");
  FS.mkdir("/home/web_user");
 },
 createDefaultDevices: function() {
  FS.mkdir("/dev");
  FS.registerDevice(FS.makedev(1, 3), {
   read: function() {
    return 0;
   },
   write: function(stream, buffer, offset, length, pos) {
    return length;
   }
  });
  FS.mkdev("/dev/null", FS.makedev(1, 3));
  TTY.register(FS.makedev(5, 0), TTY.default_tty_ops);
  TTY.register(FS.makedev(6, 0), TTY.default_tty1_ops);
  FS.mkdev("/dev/tty", FS.makedev(5, 0));
  FS.mkdev("/dev/tty1", FS.makedev(6, 0));
  var random_device = getRandomDevice();
  FS.createDevice("/dev", "random", random_device);
  FS.createDevice("/dev", "urandom", random_device);
  FS.mkdir("/dev/shm");
  FS.mkdir("/dev/shm/tmp");
 },
 createSpecialDirectories: function() {
  FS.mkdir("/proc");
  var proc_self = FS.mkdir("/proc/self");
  FS.mkdir("/proc/self/fd");
  FS.mount({
   mount: function() {
    var node = FS.createNode(proc_self, "fd", 16384 | 511, 73);
    node.node_ops = {
     lookup: function(parent, name) {
      var fd = +name;
      var stream = FS.getStream(fd);
      if (!stream) throw new FS.ErrnoError(8);
      var ret = {
       parent: null,
       mount: {
        mountpoint: "fake"
       },
       node_ops: {
        readlink: function() {
         return stream.path;
        }
       }
      };
      ret.parent = ret;
      return ret;
     }
    };
    return node;
   }
  }, {}, "/proc/self/fd");
 },
 createStandardStreams: function() {
  if (Module["stdin"]) {
   FS.createDevice("/dev", "stdin", Module["stdin"]);
  } else {
   FS.symlink("/dev/tty", "/dev/stdin");
  }
  if (Module["stdout"]) {
   FS.createDevice("/dev", "stdout", null, Module["stdout"]);
  } else {
   FS.symlink("/dev/tty", "/dev/stdout");
  }
  if (Module["stderr"]) {
   FS.createDevice("/dev", "stderr", null, Module["stderr"]);
  } else {
   FS.symlink("/dev/tty1", "/dev/stderr");
  }
  var stdin = FS.open("/dev/stdin", 0);
  var stdout = FS.open("/dev/stdout", 1);
  var stderr = FS.open("/dev/stderr", 1);
  assert(stdin.fd === 0, "invalid handle for stdin (" + stdin.fd + ")");
  assert(stdout.fd === 1, "invalid handle for stdout (" + stdout.fd + ")");
  assert(stderr.fd === 2, "invalid handle for stderr (" + stderr.fd + ")");
 },
 ensureErrnoError: function() {
  if (FS.ErrnoError) return;
  FS.ErrnoError = function ErrnoError(errno, node) {
   this.node = node;
   this.setErrno = function(errno) {
    this.errno = errno;
    for (var key in ERRNO_CODES) {
     if (ERRNO_CODES[key] === errno) {
      this.code = key;
      break;
     }
    }
   };
   this.setErrno(errno);
   this.message = ERRNO_MESSAGES[errno];
   if (this.stack) {
    Object.defineProperty(this, "stack", {
     value: new Error().stack,
     writable: true
    });
    this.stack = demangleAll(this.stack);
   }
  };
  FS.ErrnoError.prototype = new Error();
  FS.ErrnoError.prototype.constructor = FS.ErrnoError;
  [ 44 ].forEach(function(code) {
   FS.genericErrors[code] = new FS.ErrnoError(code);
   FS.genericErrors[code].stack = "<generic error, no stack>";
  });
 },
 staticInit: function() {
  FS.ensureErrnoError();
  FS.nameTable = new Array(4096);
  FS.mount(MEMFS, {}, "/");
  FS.createDefaultDirectories();
  FS.createDefaultDevices();
  FS.createSpecialDirectories();
  FS.filesystems = {
   "MEMFS": MEMFS
  };
 },
 init: function(input, output, error) {
  assert(!FS.init.initialized, "FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)");
  FS.init.initialized = true;
  FS.ensureErrnoError();
  Module["stdin"] = input || Module["stdin"];
  Module["stdout"] = output || Module["stdout"];
  Module["stderr"] = error || Module["stderr"];
  FS.createStandardStreams();
 },
 quit: function() {
  FS.init.initialized = false;
  var fflush = Module["_fflush"];
  if (fflush) fflush(0);
  for (var i = 0; i < FS.streams.length; i++) {
   var stream = FS.streams[i];
   if (!stream) {
    continue;
   }
   FS.close(stream);
  }
 },
 getMode: function(canRead, canWrite) {
  var mode = 0;
  if (canRead) mode |= 292 | 73;
  if (canWrite) mode |= 146;
  return mode;
 },
 findObject: function(path, dontResolveLastLink) {
  var ret = FS.analyzePath(path, dontResolveLastLink);
  if (ret.exists) {
   return ret.object;
  } else {
   return null;
  }
 },
 analyzePath: function(path, dontResolveLastLink) {
  try {
   var lookup = FS.lookupPath(path, {
    follow: !dontResolveLastLink
   });
   path = lookup.path;
  } catch (e) {}
  var ret = {
   isRoot: false,
   exists: false,
   error: 0,
   name: null,
   path: null,
   object: null,
   parentExists: false,
   parentPath: null,
   parentObject: null
  };
  try {
   var lookup = FS.lookupPath(path, {
    parent: true
   });
   ret.parentExists = true;
   ret.parentPath = lookup.path;
   ret.parentObject = lookup.node;
   ret.name = PATH.basename(path);
   lookup = FS.lookupPath(path, {
    follow: !dontResolveLastLink
   });
   ret.exists = true;
   ret.path = lookup.path;
   ret.object = lookup.node;
   ret.name = lookup.node.name;
   ret.isRoot = lookup.path === "/";
  } catch (e) {
   ret.error = e.errno;
  }
  return ret;
 },
 createPath: function(parent, path, canRead, canWrite) {
  parent = typeof parent === "string" ? parent : FS.getPath(parent);
  var parts = path.split("/").reverse();
  while (parts.length) {
   var part = parts.pop();
   if (!part) continue;
   var current = PATH.join2(parent, part);
   try {
    FS.mkdir(current);
   } catch (e) {}
   parent = current;
  }
  return current;
 },
 createFile: function(parent, name, properties, canRead, canWrite) {
  var path = PATH.join2(typeof parent === "string" ? parent : FS.getPath(parent), name);
  var mode = FS.getMode(canRead, canWrite);
  return FS.create(path, mode);
 },
 createDataFile: function(parent, name, data, canRead, canWrite, canOwn) {
  var path = name ? PATH.join2(typeof parent === "string" ? parent : FS.getPath(parent), name) : parent;
  var mode = FS.getMode(canRead, canWrite);
  var node = FS.create(path, mode);
  if (data) {
   if (typeof data === "string") {
    var arr = new Array(data.length);
    for (var i = 0, len = data.length; i < len; ++i) arr[i] = data.charCodeAt(i);
    data = arr;
   }
   FS.chmod(node, mode | 146);
   var stream = FS.open(node, 577);
   FS.write(stream, data, 0, data.length, 0, canOwn);
   FS.close(stream);
   FS.chmod(node, mode);
  }
  return node;
 },
 createDevice: function(parent, name, input, output) {
  var path = PATH.join2(typeof parent === "string" ? parent : FS.getPath(parent), name);
  var mode = FS.getMode(!!input, !!output);
  if (!FS.createDevice.major) FS.createDevice.major = 64;
  var dev = FS.makedev(FS.createDevice.major++, 0);
  FS.registerDevice(dev, {
   open: function(stream) {
    stream.seekable = false;
   },
   close: function(stream) {
    if (output && output.buffer && output.buffer.length) {
     output(10);
    }
   },
   read: function(stream, buffer, offset, length, pos) {
    var bytesRead = 0;
    for (var i = 0; i < length; i++) {
     var result;
     try {
      result = input();
     } catch (e) {
      throw new FS.ErrnoError(29);
     }
     if (result === undefined && bytesRead === 0) {
      throw new FS.ErrnoError(6);
     }
     if (result === null || result === undefined) break;
     bytesRead++;
     buffer[offset + i] = result;
    }
    if (bytesRead) {
     stream.node.timestamp = Date.now();
    }
    return bytesRead;
   },
   write: function(stream, buffer, offset, length, pos) {
    for (var i = 0; i < length; i++) {
     try {
      output(buffer[offset + i]);
     } catch (e) {
      throw new FS.ErrnoError(29);
     }
    }
    if (length) {
     stream.node.timestamp = Date.now();
    }
    return i;
   }
  });
  return FS.mkdev(path, mode, dev);
 },
 forceLoadFile: function(obj) {
  if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
  if (typeof XMLHttpRequest !== "undefined") {
   throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
  } else if (read_) {
   try {
    obj.contents = intArrayFromString(read_(obj.url), true);
    obj.usedBytes = obj.contents.length;
   } catch (e) {
    throw new FS.ErrnoError(29);
   }
  } else {
   throw new Error("Cannot load without read() or XMLHttpRequest.");
  }
 },
 createLazyFile: function(parent, name, url, canRead, canWrite) {
  function LazyUint8Array() {
   this.lengthKnown = false;
   this.chunks = [];
  }
  LazyUint8Array.prototype.get = function LazyUint8Array_get(idx) {
   if (idx > this.length - 1 || idx < 0) {
    return undefined;
   }
   var chunkOffset = idx % this.chunkSize;
   var chunkNum = idx / this.chunkSize | 0;
   return this.getter(chunkNum)[chunkOffset];
  };
  LazyUint8Array.prototype.setDataGetter = function LazyUint8Array_setDataGetter(getter) {
   this.getter = getter;
  };
  LazyUint8Array.prototype.cacheLength = function LazyUint8Array_cacheLength() {
   var xhr = new XMLHttpRequest();
   xhr.open("HEAD", url, false);
   xhr.send(null);
   if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
   var datalength = Number(xhr.getResponseHeader("Content-length"));
   var header;
   var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
   var usesGzip = (header = xhr.getResponseHeader("Content-Encoding")) && header === "gzip";
   var chunkSize = 1024 * 1024;
   if (!hasByteServing) chunkSize = datalength;
   var doXHR = function(from, to) {
    if (from > to) throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
    if (to > datalength - 1) throw new Error("only " + datalength + " bytes available! programmer error!");
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, false);
    if (datalength !== chunkSize) xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
    if (typeof Uint8Array != "undefined") xhr.responseType = "arraybuffer";
    if (xhr.overrideMimeType) {
     xhr.overrideMimeType("text/plain; charset=x-user-defined");
    }
    xhr.send(null);
    if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
    if (xhr.response !== undefined) {
     return new Uint8Array(xhr.response || []);
    } else {
     return intArrayFromString(xhr.responseText || "", true);
    }
   };
   var lazyArray = this;
   lazyArray.setDataGetter(function(chunkNum) {
    var start = chunkNum * chunkSize;
    var end = (chunkNum + 1) * chunkSize - 1;
    end = Math.min(end, datalength - 1);
    if (typeof lazyArray.chunks[chunkNum] === "undefined") {
     lazyArray.chunks[chunkNum] = doXHR(start, end);
    }
    if (typeof lazyArray.chunks[chunkNum] === "undefined") throw new Error("doXHR failed!");
    return lazyArray.chunks[chunkNum];
   });
   if (usesGzip || !datalength) {
    chunkSize = datalength = 1;
    datalength = this.getter(0).length;
    chunkSize = datalength;
    out("LazyFiles on gzip forces download of the whole file when length is accessed");
   }
   this._length = datalength;
   this._chunkSize = chunkSize;
   this.lengthKnown = true;
  };
  if (typeof XMLHttpRequest !== "undefined") {
   if (!ENVIRONMENT_IS_WORKER) throw "Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";
   var lazyArray = new LazyUint8Array();
   Object.defineProperties(lazyArray, {
    length: {
     get: function() {
      if (!this.lengthKnown) {
       this.cacheLength();
      }
      return this._length;
     }
    },
    chunkSize: {
     get: function() {
      if (!this.lengthKnown) {
       this.cacheLength();
      }
      return this._chunkSize;
     }
    }
   });
   var properties = {
    isDevice: false,
    contents: lazyArray
   };
  } else {
   var properties = {
    isDevice: false,
    url: url
   };
  }
  var node = FS.createFile(parent, name, properties, canRead, canWrite);
  if (properties.contents) {
   node.contents = properties.contents;
  } else if (properties.url) {
   node.contents = null;
   node.url = properties.url;
  }
  Object.defineProperties(node, {
   usedBytes: {
    get: function() {
     return this.contents.length;
    }
   }
  });
  var stream_ops = {};
  var keys = Object.keys(node.stream_ops);
  keys.forEach(function(key) {
   var fn = node.stream_ops[key];
   stream_ops[key] = function forceLoadLazyFile() {
    FS.forceLoadFile(node);
    return fn.apply(null, arguments);
   };
  });
  stream_ops.read = function stream_ops_read(stream, buffer, offset, length, position) {
   FS.forceLoadFile(node);
   var contents = stream.node.contents;
   if (position >= contents.length) return 0;
   var size = Math.min(contents.length - position, length);
   assert(size >= 0);
   if (contents.slice) {
    for (var i = 0; i < size; i++) {
     buffer[offset + i] = contents[position + i];
    }
   } else {
    for (var i = 0; i < size; i++) {
     buffer[offset + i] = contents.get(position + i);
    }
   }
   return size;
  };
  node.stream_ops = stream_ops;
  return node;
 },
 createPreloadedFile: function(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) {
  Browser.init();
  var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
  var dep = getUniqueRunDependency("cp " + fullname);
  function processData(byteArray) {
   function finish(byteArray) {
    if (preFinish) preFinish();
    if (!dontCreateFile) {
     FS.createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
    }
    if (onload) onload();
    removeRunDependency(dep);
   }
   var handled = false;
   Module["preloadPlugins"].forEach(function(plugin) {
    if (handled) return;
    if (plugin["canHandle"](fullname)) {
     plugin["handle"](byteArray, fullname, finish, function() {
      if (onerror) onerror();
      removeRunDependency(dep);
     });
     handled = true;
    }
   });
   if (!handled) finish(byteArray);
  }
  addRunDependency(dep);
  if (typeof url == "string") {
   asyncLoad(url, function(byteArray) {
    processData(byteArray);
   }, onerror);
  } else {
   processData(url);
  }
 },
 indexedDB: function() {
  return window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
 },
 DB_NAME: function() {
  return "EM_FS_" + window.location.pathname;
 },
 DB_VERSION: 20,
 DB_STORE_NAME: "FILE_DATA",
 saveFilesToDB: function(paths, onload, onerror) {
  onload = onload || function() {};
  onerror = onerror || function() {};
  var indexedDB = FS.indexedDB();
  try {
   var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION);
  } catch (e) {
   return onerror(e);
  }
  openRequest.onupgradeneeded = function openRequest_onupgradeneeded() {
   out("creating db");
   var db = openRequest.result;
   db.createObjectStore(FS.DB_STORE_NAME);
  };
  openRequest.onsuccess = function openRequest_onsuccess() {
   var db = openRequest.result;
   var transaction = db.transaction([ FS.DB_STORE_NAME ], "readwrite");
   var files = transaction.objectStore(FS.DB_STORE_NAME);
   var ok = 0, fail = 0, total = paths.length;
   function finish() {
    if (fail == 0) onload(); else onerror();
   }
   paths.forEach(function(path) {
    var putRequest = files.put(FS.analyzePath(path).object.contents, path);
    putRequest.onsuccess = function putRequest_onsuccess() {
     ok++;
     if (ok + fail == total) finish();
    };
    putRequest.onerror = function putRequest_onerror() {
     fail++;
     if (ok + fail == total) finish();
    };
   });
   transaction.onerror = onerror;
  };
  openRequest.onerror = onerror;
 },
 loadFilesFromDB: function(paths, onload, onerror) {
  onload = onload || function() {};
  onerror = onerror || function() {};
  var indexedDB = FS.indexedDB();
  try {
   var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION);
  } catch (e) {
   return onerror(e);
  }
  openRequest.onupgradeneeded = onerror;
  openRequest.onsuccess = function openRequest_onsuccess() {
   var db = openRequest.result;
   try {
    var transaction = db.transaction([ FS.DB_STORE_NAME ], "readonly");
   } catch (e) {
    onerror(e);
    return;
   }
   var files = transaction.objectStore(FS.DB_STORE_NAME);
   var ok = 0, fail = 0, total = paths.length;
   function finish() {
    if (fail == 0) onload(); else onerror();
   }
   paths.forEach(function(path) {
    var getRequest = files.get(path);
    getRequest.onsuccess = function getRequest_onsuccess() {
     if (FS.analyzePath(path).exists) {
      FS.unlink(path);
     }
     FS.createDataFile(PATH.dirname(path), PATH.basename(path), getRequest.result, true, true, true);
     ok++;
     if (ok + fail == total) finish();
    };
    getRequest.onerror = function getRequest_onerror() {
     fail++;
     if (ok + fail == total) finish();
    };
   });
   transaction.onerror = onerror;
  };
  openRequest.onerror = onerror;
 },
 absolutePath: function() {
  abort("FS.absolutePath has been removed; use PATH_FS.resolve instead");
 },
 createFolder: function() {
  abort("FS.createFolder has been removed; use FS.mkdir instead");
 },
 createLink: function() {
  abort("FS.createLink has been removed; use FS.symlink instead");
 },
 joinPath: function() {
  abort("FS.joinPath has been removed; use PATH.join instead");
 },
 mmapAlloc: function() {
  abort("FS.mmapAlloc has been replaced by the top level function mmapAlloc");
 },
 standardizePath: function() {
  abort("FS.standardizePath has been removed; use PATH.normalize instead");
 }
};

function _emscripten_set_main_loop_timing(mode, value) {
 Browser.mainLoop.timingMode = mode;
 Browser.mainLoop.timingValue = value;
 if (!Browser.mainLoop.func) {
  err("emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call emscripten_set_main_loop first to set one up.");
  return 1;
 }
 if (!Browser.mainLoop.running) {
  runtimeKeepalivePush();
  Browser.mainLoop.running = true;
 }
 if (mode == 0) {
  Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setTimeout() {
   var timeUntilNextTick = Math.max(0, Browser.mainLoop.tickStartTime + value - _emscripten_get_now()) | 0;
   setTimeout(Browser.mainLoop.runner, timeUntilNextTick);
  };
  Browser.mainLoop.method = "timeout";
 } else if (mode == 1) {
  Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_rAF() {
   Browser.requestAnimationFrame(Browser.mainLoop.runner);
  };
  Browser.mainLoop.method = "rAF";
 } else if (mode == 2) {
  if (typeof setImmediate === "undefined") {
   var setImmediates = [];
   var emscriptenMainLoopMessageId = "setimmediate";
   var Browser_setImmediate_messageHandler = function(event) {
    if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
     event.stopPropagation();
     setImmediates.shift()();
    }
   };
   addEventListener("message", Browser_setImmediate_messageHandler, true);
   setImmediate = function Browser_emulated_setImmediate(func) {
    setImmediates.push(func);
    if (ENVIRONMENT_IS_WORKER) {
     if (Module["setImmediates"] === undefined) Module["setImmediates"] = [];
     Module["setImmediates"].push(func);
     postMessage({
      target: emscriptenMainLoopMessageId
     });
    } else postMessage(emscriptenMainLoopMessageId, "*");
   };
  }
  Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setImmediate() {
   setImmediate(Browser.mainLoop.runner);
  };
  Browser.mainLoop.method = "immediate";
 }
 return 0;
}

var _emscripten_get_now;

if (ENVIRONMENT_IS_NODE) {
 _emscripten_get_now = function() {
  var t = process["hrtime"]();
  return t[0] * 1e3 + t[1] / 1e6;
 };
} else _emscripten_get_now = function() {
 return performance.now();
};

function runtimeKeepalivePush() {
 runtimeKeepaliveCounter += 1;
}

function _exit(status) {
 exit(status);
}

function maybeExit() {
 if (!keepRuntimeAlive()) {
  try {
   _exit(EXITSTATUS);
  } catch (e) {
   handleException(e);
  }
 }
}

function setMainLoop(browserIterationFunc, fps, simulateInfiniteLoop, arg, noSetTiming) {
 assert(!Browser.mainLoop.func, "emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.");
 Browser.mainLoop.func = browserIterationFunc;
 Browser.mainLoop.arg = arg;
 var thisMainLoopId = Browser.mainLoop.currentlyRunningMainloop;
 function checkIsRunning() {
  if (thisMainLoopId < Browser.mainLoop.currentlyRunningMainloop) {
   runtimeKeepalivePop();
   maybeExit();
   return false;
  }
  return true;
 }
 Browser.mainLoop.running = false;
 Browser.mainLoop.runner = function Browser_mainLoop_runner() {
  if (ABORT) return;
  if (Browser.mainLoop.queue.length > 0) {
   var start = Date.now();
   var blocker = Browser.mainLoop.queue.shift();
   blocker.func(blocker.arg);
   if (Browser.mainLoop.remainingBlockers) {
    var remaining = Browser.mainLoop.remainingBlockers;
    var next = remaining % 1 == 0 ? remaining - 1 : Math.floor(remaining);
    if (blocker.counted) {
     Browser.mainLoop.remainingBlockers = next;
    } else {
     next = next + .5;
     Browser.mainLoop.remainingBlockers = (8 * remaining + next) / 9;
    }
   }
   out('main loop blocker "' + blocker.name + '" took ' + (Date.now() - start) + " ms");
   Browser.mainLoop.updateStatus();
   if (!checkIsRunning()) return;
   setTimeout(Browser.mainLoop.runner, 0);
   return;
  }
  if (!checkIsRunning()) return;
  Browser.mainLoop.currentFrameNumber = Browser.mainLoop.currentFrameNumber + 1 | 0;
  if (Browser.mainLoop.timingMode == 1 && Browser.mainLoop.timingValue > 1 && Browser.mainLoop.currentFrameNumber % Browser.mainLoop.timingValue != 0) {
   Browser.mainLoop.scheduler();
   return;
  } else if (Browser.mainLoop.timingMode == 0) {
   Browser.mainLoop.tickStartTime = _emscripten_get_now();
  }
  if (Browser.mainLoop.method === "timeout" && Module.ctx) {
   warnOnce("Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!");
   Browser.mainLoop.method = "";
  }
  Browser.mainLoop.runIter(browserIterationFunc);
  checkStackCookie();
  if (!checkIsRunning()) return;
  if (typeof SDL === "object" && SDL.audio && SDL.audio.queueNewAudioData) SDL.audio.queueNewAudioData();
  Browser.mainLoop.scheduler();
 };
 if (!noSetTiming) {
  if (fps && fps > 0) _emscripten_set_main_loop_timing(0, 1e3 / fps); else _emscripten_set_main_loop_timing(1, 1);
  Browser.mainLoop.scheduler();
 }
 if (simulateInfiniteLoop) {
  throw "unwind";
 }
}

function callUserCallback(func, synchronous) {
 if (ABORT) {
  err("user callback triggered after application aborted.  Ignoring.");
  return;
 }
 if (synchronous) {
  func();
  return;
 }
 try {
  func();
  maybeExit();
 } catch (e) {
  handleException(e);
 }
}

function runtimeKeepalivePop() {
 assert(runtimeKeepaliveCounter > 0);
 runtimeKeepaliveCounter -= 1;
}

function safeSetTimeout(func, timeout) {
 runtimeKeepalivePush();
 return setTimeout(function() {
  runtimeKeepalivePop();
  callUserCallback(func);
 }, timeout);
}

var Browser = {
 mainLoop: {
  running: false,
  scheduler: null,
  method: "",
  currentlyRunningMainloop: 0,
  func: null,
  arg: 0,
  timingMode: 0,
  timingValue: 0,
  currentFrameNumber: 0,
  queue: [],
  pause: function() {
   Browser.mainLoop.scheduler = null;
   Browser.mainLoop.currentlyRunningMainloop++;
  },
  resume: function() {
   Browser.mainLoop.currentlyRunningMainloop++;
   var timingMode = Browser.mainLoop.timingMode;
   var timingValue = Browser.mainLoop.timingValue;
   var func = Browser.mainLoop.func;
   Browser.mainLoop.func = null;
   setMainLoop(func, 0, false, Browser.mainLoop.arg, true);
   _emscripten_set_main_loop_timing(timingMode, timingValue);
   Browser.mainLoop.scheduler();
  },
  updateStatus: function() {
   if (Module["setStatus"]) {
    var message = Module["statusMessage"] || "Please wait...";
    var remaining = Browser.mainLoop.remainingBlockers;
    var expected = Browser.mainLoop.expectedBlockers;
    if (remaining) {
     if (remaining < expected) {
      Module["setStatus"](message + " (" + (expected - remaining) + "/" + expected + ")");
     } else {
      Module["setStatus"](message);
     }
    } else {
     Module["setStatus"]("");
    }
   }
  },
  runIter: function(func) {
   if (ABORT) return;
   if (Module["preMainLoop"]) {
    var preRet = Module["preMainLoop"]();
    if (preRet === false) {
     return;
    }
   }
   callUserCallback(func);
   if (Module["postMainLoop"]) Module["postMainLoop"]();
  }
 },
 isFullscreen: false,
 pointerLock: false,
 moduleContextCreatedCallbacks: [],
 workers: [],
 init: function() {
  if (!Module["preloadPlugins"]) Module["preloadPlugins"] = [];
  if (Browser.initted) return;
  Browser.initted = true;
  try {
   new Blob();
   Browser.hasBlobConstructor = true;
  } catch (e) {
   Browser.hasBlobConstructor = false;
   out("warning: no blob constructor, cannot create blobs with mimetypes");
  }
  Browser.BlobBuilder = typeof MozBlobBuilder != "undefined" ? MozBlobBuilder : typeof WebKitBlobBuilder != "undefined" ? WebKitBlobBuilder : !Browser.hasBlobConstructor ? out("warning: no BlobBuilder") : null;
  Browser.URLObject = typeof window != "undefined" ? window.URL ? window.URL : window.webkitURL : undefined;
  if (!Module.noImageDecoding && typeof Browser.URLObject === "undefined") {
   out("warning: Browser does not support creating object URLs. Built-in browser image decoding will not be available.");
   Module.noImageDecoding = true;
  }
  var imagePlugin = {};
  imagePlugin["canHandle"] = function imagePlugin_canHandle(name) {
   return !Module.noImageDecoding && /\.(jpg|jpeg|png|bmp)$/i.test(name);
  };
  imagePlugin["handle"] = function imagePlugin_handle(byteArray, name, onload, onerror) {
   var b = null;
   if (Browser.hasBlobConstructor) {
    try {
     b = new Blob([ byteArray ], {
      type: Browser.getMimetype(name)
     });
     if (b.size !== byteArray.length) {
      b = new Blob([ new Uint8Array(byteArray).buffer ], {
       type: Browser.getMimetype(name)
      });
     }
    } catch (e) {
     warnOnce("Blob constructor present but fails: " + e + "; falling back to blob builder");
    }
   }
   if (!b) {
    var bb = new Browser.BlobBuilder();
    bb.append(new Uint8Array(byteArray).buffer);
    b = bb.getBlob();
   }
   var url = Browser.URLObject.createObjectURL(b);
   assert(typeof url == "string", "createObjectURL must return a url as a string");
   var img = new Image();
   img.onload = function img_onload() {
    assert(img.complete, "Image " + name + " could not be decoded");
    var canvas = document.createElement("canvas");
    canvas.width = img.width;
    canvas.height = img.height;
    var ctx = canvas.getContext("2d");
    ctx.drawImage(img, 0, 0);
    Module["preloadedImages"][name] = canvas;
    Browser.URLObject.revokeObjectURL(url);
    if (onload) onload(byteArray);
   };
   img.onerror = function img_onerror(event) {
    out("Image " + url + " could not be decoded");
    if (onerror) onerror();
   };
   img.src = url;
  };
  Module["preloadPlugins"].push(imagePlugin);
  var audioPlugin = {};
  audioPlugin["canHandle"] = function audioPlugin_canHandle(name) {
   return !Module.noAudioDecoding && name.substr(-4) in {
    ".ogg": 1,
    ".wav": 1,
    ".mp3": 1
   };
  };
  audioPlugin["handle"] = function audioPlugin_handle(byteArray, name, onload, onerror) {
   var done = false;
   function finish(audio) {
    if (done) return;
    done = true;
    Module["preloadedAudios"][name] = audio;
    if (onload) onload(byteArray);
   }
   function fail() {
    if (done) return;
    done = true;
    Module["preloadedAudios"][name] = new Audio();
    if (onerror) onerror();
   }
   if (Browser.hasBlobConstructor) {
    try {
     var b = new Blob([ byteArray ], {
      type: Browser.getMimetype(name)
     });
    } catch (e) {
     return fail();
    }
    var url = Browser.URLObject.createObjectURL(b);
    assert(typeof url == "string", "createObjectURL must return a url as a string");
    var audio = new Audio();
    audio.addEventListener("canplaythrough", function() {
     finish(audio);
    }, false);
    audio.onerror = function audio_onerror(event) {
     if (done) return;
     out("warning: browser could not fully decode audio " + name + ", trying slower base64 approach");
     function encode64(data) {
      var BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
      var PAD = "=";
      var ret = "";
      var leftchar = 0;
      var leftbits = 0;
      for (var i = 0; i < data.length; i++) {
       leftchar = leftchar << 8 | data[i];
       leftbits += 8;
       while (leftbits >= 6) {
        var curr = leftchar >> leftbits - 6 & 63;
        leftbits -= 6;
        ret += BASE[curr];
       }
      }
      if (leftbits == 2) {
       ret += BASE[(leftchar & 3) << 4];
       ret += PAD + PAD;
      } else if (leftbits == 4) {
       ret += BASE[(leftchar & 15) << 2];
       ret += PAD;
      }
      return ret;
     }
     audio.src = "data:audio/x-" + name.substr(-3) + ";base64," + encode64(byteArray);
     finish(audio);
    };
    audio.src = url;
    safeSetTimeout(function() {
     finish(audio);
    }, 1e4);
   } else {
    return fail();
   }
  };
  Module["preloadPlugins"].push(audioPlugin);
  function pointerLockChange() {
   Browser.pointerLock = document["pointerLockElement"] === Module["canvas"] || document["mozPointerLockElement"] === Module["canvas"] || document["webkitPointerLockElement"] === Module["canvas"] || document["msPointerLockElement"] === Module["canvas"];
  }
  var canvas = Module["canvas"];
  if (canvas) {
   canvas.requestPointerLock = canvas["requestPointerLock"] || canvas["mozRequestPointerLock"] || canvas["webkitRequestPointerLock"] || canvas["msRequestPointerLock"] || function() {};
   canvas.exitPointerLock = document["exitPointerLock"] || document["mozExitPointerLock"] || document["webkitExitPointerLock"] || document["msExitPointerLock"] || function() {};
   canvas.exitPointerLock = canvas.exitPointerLock.bind(document);
   document.addEventListener("pointerlockchange", pointerLockChange, false);
   document.addEventListener("mozpointerlockchange", pointerLockChange, false);
   document.addEventListener("webkitpointerlockchange", pointerLockChange, false);
   document.addEventListener("mspointerlockchange", pointerLockChange, false);
   if (Module["elementPointerLock"]) {
    canvas.addEventListener("click", function(ev) {
     if (!Browser.pointerLock && Module["canvas"].requestPointerLock) {
      Module["canvas"].requestPointerLock();
      ev.preventDefault();
     }
    }, false);
   }
  }
 },
 createContext: function(canvas, useWebGL, setInModule, webGLContextAttributes) {
  if (useWebGL && Module.ctx && canvas == Module.canvas) return Module.ctx;
  var ctx;
  var contextHandle;
  if (useWebGL) {
   var contextAttributes = {
    antialias: false,
    alpha: false,
    majorVersion: 1
   };
   if (webGLContextAttributes) {
    for (var attribute in webGLContextAttributes) {
     contextAttributes[attribute] = webGLContextAttributes[attribute];
    }
   }
   if (typeof GL !== "undefined") {
    contextHandle = GL.createContext(canvas, contextAttributes);
    if (contextHandle) {
     ctx = GL.getContext(contextHandle).GLctx;
    }
   }
  } else {
   ctx = canvas.getContext("2d");
  }
  if (!ctx) return null;
  if (setInModule) {
   if (!useWebGL) assert(typeof GLctx === "undefined", "cannot set in module if GLctx is used, but we are a non-GL context that would replace it");
   Module.ctx = ctx;
   if (useWebGL) GL.makeContextCurrent(contextHandle);
   Module.useWebGL = useWebGL;
   Browser.moduleContextCreatedCallbacks.forEach(function(callback) {
    callback();
   });
   Browser.init();
  }
  return ctx;
 },
 destroyContext: function(canvas, useWebGL, setInModule) {},
 fullscreenHandlersInstalled: false,
 lockPointer: undefined,
 resizeCanvas: undefined,
 requestFullscreen: function(lockPointer, resizeCanvas) {
  Browser.lockPointer = lockPointer;
  Browser.resizeCanvas = resizeCanvas;
  if (typeof Browser.lockPointer === "undefined") Browser.lockPointer = true;
  if (typeof Browser.resizeCanvas === "undefined") Browser.resizeCanvas = false;
  var canvas = Module["canvas"];
  function fullscreenChange() {
   Browser.isFullscreen = false;
   var canvasContainer = canvas.parentNode;
   if ((document["fullscreenElement"] || document["mozFullScreenElement"] || document["msFullscreenElement"] || document["webkitFullscreenElement"] || document["webkitCurrentFullScreenElement"]) === canvasContainer) {
    canvas.exitFullscreen = Browser.exitFullscreen;
    if (Browser.lockPointer) canvas.requestPointerLock();
    Browser.isFullscreen = true;
    if (Browser.resizeCanvas) {
     Browser.setFullscreenCanvasSize();
    } else {
     Browser.updateCanvasDimensions(canvas);
    }
   } else {
    canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
    canvasContainer.parentNode.removeChild(canvasContainer);
    if (Browser.resizeCanvas) {
     Browser.setWindowedCanvasSize();
    } else {
     Browser.updateCanvasDimensions(canvas);
    }
   }
   if (Module["onFullScreen"]) Module["onFullScreen"](Browser.isFullscreen);
   if (Module["onFullscreen"]) Module["onFullscreen"](Browser.isFullscreen);
  }
  if (!Browser.fullscreenHandlersInstalled) {
   Browser.fullscreenHandlersInstalled = true;
   document.addEventListener("fullscreenchange", fullscreenChange, false);
   document.addEventListener("mozfullscreenchange", fullscreenChange, false);
   document.addEventListener("webkitfullscreenchange", fullscreenChange, false);
   document.addEventListener("MSFullscreenChange", fullscreenChange, false);
  }
  var canvasContainer = document.createElement("div");
  canvas.parentNode.insertBefore(canvasContainer, canvas);
  canvasContainer.appendChild(canvas);
  canvasContainer.requestFullscreen = canvasContainer["requestFullscreen"] || canvasContainer["mozRequestFullScreen"] || canvasContainer["msRequestFullscreen"] || (canvasContainer["webkitRequestFullscreen"] ? function() {
   canvasContainer["webkitRequestFullscreen"](Element["ALLOW_KEYBOARD_INPUT"]);
  } : null) || (canvasContainer["webkitRequestFullScreen"] ? function() {
   canvasContainer["webkitRequestFullScreen"](Element["ALLOW_KEYBOARD_INPUT"]);
  } : null);
  canvasContainer.requestFullscreen();
 },
 requestFullScreen: function() {
  abort("Module.requestFullScreen has been replaced by Module.requestFullscreen (without a capital S)");
 },
 exitFullscreen: function() {
  if (!Browser.isFullscreen) {
   return false;
  }
  var CFS = document["exitFullscreen"] || document["cancelFullScreen"] || document["mozCancelFullScreen"] || document["msExitFullscreen"] || document["webkitCancelFullScreen"] || function() {};
  CFS.apply(document, []);
  return true;
 },
 nextRAF: 0,
 fakeRequestAnimationFrame: function(func) {
  var now = Date.now();
  if (Browser.nextRAF === 0) {
   Browser.nextRAF = now + 1e3 / 60;
  } else {
   while (now + 2 >= Browser.nextRAF) {
    Browser.nextRAF += 1e3 / 60;
   }
  }
  var delay = Math.max(Browser.nextRAF - now, 0);
  setTimeout(func, delay);
 },
 requestAnimationFrame: function(func) {
  if (typeof requestAnimationFrame === "function") {
   requestAnimationFrame(func);
   return;
  }
  var RAF = Browser.fakeRequestAnimationFrame;
  RAF(func);
 },
 safeSetTimeout: function(func) {
  return safeSetTimeout(func);
 },
 safeRequestAnimationFrame: function(func) {
  runtimeKeepalivePush();
  return Browser.requestAnimationFrame(function() {
   runtimeKeepalivePop();
   callUserCallback(func);
  });
 },
 getMimetype: function(name) {
  return {
   "jpg": "image/jpeg",
   "jpeg": "image/jpeg",
   "png": "image/png",
   "bmp": "image/bmp",
   "ogg": "audio/ogg",
   "wav": "audio/wav",
   "mp3": "audio/mpeg"
  }[name.substr(name.lastIndexOf(".") + 1)];
 },
 getUserMedia: function(func) {
  if (!window.getUserMedia) {
   window.getUserMedia = navigator["getUserMedia"] || navigator["mozGetUserMedia"];
  }
  window.getUserMedia(func);
 },
 getMovementX: function(event) {
  return event["movementX"] || event["mozMovementX"] || event["webkitMovementX"] || 0;
 },
 getMovementY: function(event) {
  return event["movementY"] || event["mozMovementY"] || event["webkitMovementY"] || 0;
 },
 getMouseWheelDelta: function(event) {
  var delta = 0;
  switch (event.type) {
  case "DOMMouseScroll":
   delta = event.detail / 3;
   break;

  case "mousewheel":
   delta = event.wheelDelta / 120;
   break;

  case "wheel":
   delta = event.deltaY;
   switch (event.deltaMode) {
   case 0:
    delta /= 100;
    break;

   case 1:
    delta /= 3;
    break;

   case 2:
    delta *= 80;
    break;

   default:
    throw "unrecognized mouse wheel delta mode: " + event.deltaMode;
   }
   break;

  default:
   throw "unrecognized mouse wheel event: " + event.type;
  }
  return delta;
 },
 mouseX: 0,
 mouseY: 0,
 mouseMovementX: 0,
 mouseMovementY: 0,
 touches: {},
 lastTouches: {},
 calculateMouseEvent: function(event) {
  if (Browser.pointerLock) {
   if (event.type != "mousemove" && "mozMovementX" in event) {
    Browser.mouseMovementX = Browser.mouseMovementY = 0;
   } else {
    Browser.mouseMovementX = Browser.getMovementX(event);
    Browser.mouseMovementY = Browser.getMovementY(event);
   }
   if (typeof SDL != "undefined") {
    Browser.mouseX = SDL.mouseX + Browser.mouseMovementX;
    Browser.mouseY = SDL.mouseY + Browser.mouseMovementY;
   } else {
    Browser.mouseX += Browser.mouseMovementX;
    Browser.mouseY += Browser.mouseMovementY;
   }
  } else {
   var rect = Module["canvas"].getBoundingClientRect();
   var cw = Module["canvas"].width;
   var ch = Module["canvas"].height;
   var scrollX = typeof window.scrollX !== "undefined" ? window.scrollX : window.pageXOffset;
   var scrollY = typeof window.scrollY !== "undefined" ? window.scrollY : window.pageYOffset;
   assert(typeof scrollX !== "undefined" && typeof scrollY !== "undefined", "Unable to retrieve scroll position, mouse positions likely broken.");
   if (event.type === "touchstart" || event.type === "touchend" || event.type === "touchmove") {
    var touch = event.touch;
    if (touch === undefined) {
     return;
    }
    var adjustedX = touch.pageX - (scrollX + rect.left);
    var adjustedY = touch.pageY - (scrollY + rect.top);
    adjustedX = adjustedX * (cw / rect.width);
    adjustedY = adjustedY * (ch / rect.height);
    var coords = {
     x: adjustedX,
     y: adjustedY
    };
    if (event.type === "touchstart") {
     Browser.lastTouches[touch.identifier] = coords;
     Browser.touches[touch.identifier] = coords;
    } else if (event.type === "touchend" || event.type === "touchmove") {
     var last = Browser.touches[touch.identifier];
     if (!last) last = coords;
     Browser.lastTouches[touch.identifier] = last;
     Browser.touches[touch.identifier] = coords;
    }
    return;
   }
   var x = event.pageX - (scrollX + rect.left);
   var y = event.pageY - (scrollY + rect.top);
   x = x * (cw / rect.width);
   y = y * (ch / rect.height);
   Browser.mouseMovementX = x - Browser.mouseX;
   Browser.mouseMovementY = y - Browser.mouseY;
   Browser.mouseX = x;
   Browser.mouseY = y;
  }
 },
 resizeListeners: [],
 updateResizeListeners: function() {
  var canvas = Module["canvas"];
  Browser.resizeListeners.forEach(function(listener) {
   listener(canvas.width, canvas.height);
  });
 },
 setCanvasSize: function(width, height, noUpdates) {
  var canvas = Module["canvas"];
  Browser.updateCanvasDimensions(canvas, width, height);
  if (!noUpdates) Browser.updateResizeListeners();
 },
 windowedWidth: 0,
 windowedHeight: 0,
 setFullscreenCanvasSize: function() {
  if (typeof SDL != "undefined") {
   var flags = _asan_js_load_4u(SDL.screen >> 2);
   flags = flags | 8388608;
   _asan_js_store_4(SDL.screen >> 2, flags);
  }
  Browser.updateCanvasDimensions(Module["canvas"]);
  Browser.updateResizeListeners();
 },
 setWindowedCanvasSize: function() {
  if (typeof SDL != "undefined") {
   var flags = _asan_js_load_4u(SDL.screen >> 2);
   flags = flags & ~8388608;
   _asan_js_store_4(SDL.screen >> 2, flags);
  }
  Browser.updateCanvasDimensions(Module["canvas"]);
  Browser.updateResizeListeners();
 },
 updateCanvasDimensions: function(canvas, wNative, hNative) {
  if (wNative && hNative) {
   canvas.widthNative = wNative;
   canvas.heightNative = hNative;
  } else {
   wNative = canvas.widthNative;
   hNative = canvas.heightNative;
  }
  var w = wNative;
  var h = hNative;
  if (Module["forcedAspectRatio"] && Module["forcedAspectRatio"] > 0) {
   if (w / h < Module["forcedAspectRatio"]) {
    w = Math.round(h * Module["forcedAspectRatio"]);
   } else {
    h = Math.round(w / Module["forcedAspectRatio"]);
   }
  }
  if ((document["fullscreenElement"] || document["mozFullScreenElement"] || document["msFullscreenElement"] || document["webkitFullscreenElement"] || document["webkitCurrentFullScreenElement"]) === canvas.parentNode && typeof screen != "undefined") {
   var factor = Math.min(screen.width / w, screen.height / h);
   w = Math.round(w * factor);
   h = Math.round(h * factor);
  }
  if (Browser.resizeCanvas) {
   if (canvas.width != w) canvas.width = w;
   if (canvas.height != h) canvas.height = h;
   if (typeof canvas.style != "undefined") {
    canvas.style.removeProperty("width");
    canvas.style.removeProperty("height");
   }
  } else {
   if (canvas.width != wNative) canvas.width = wNative;
   if (canvas.height != hNative) canvas.height = hNative;
   if (typeof canvas.style != "undefined") {
    if (w != wNative || h != hNative) {
     canvas.style.setProperty("width", w + "px", "important");
     canvas.style.setProperty("height", h + "px", "important");
    } else {
     canvas.style.removeProperty("width");
     canvas.style.removeProperty("height");
    }
   }
  }
 }
};

function _SDL_GetTicks() {
 return Date.now() - SDL.startTime | 0;
}

function _SDL_LockSurface(surf) {
 var surfData = SDL.surfaces[surf];
 surfData.locked++;
 if (surfData.locked > 1) return 0;
 if (!surfData.buffer) {
  surfData.buffer = _malloc(surfData.width * surfData.height * 4);
  _asan_js_store_4(surf + 20 >> 2, surfData.buffer);
 }
 _asan_js_store_4(surf + 20 >> 2, surfData.buffer);
 if (surf == SDL.screen && Module.screenIsReadOnly && surfData.image) return 0;
 if (SDL.defaults.discardOnLock) {
  if (!surfData.image) {
   surfData.image = surfData.ctx.createImageData(surfData.width, surfData.height);
  }
  if (!SDL.defaults.opaqueFrontBuffer) return;
 } else {
  surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
 }
 if (surf == SDL.screen && SDL.defaults.opaqueFrontBuffer) {
  var data = surfData.image.data;
  var num = data.length;
  for (var i = 0; i < num / 4; i++) {
   data[i * 4 + 3] = 255;
  }
 }
 if (SDL.defaults.copyOnLock && !SDL.defaults.discardOnLock) {
  if (surfData.isFlagSet(2097152)) {
   throw "CopyOnLock is not supported for SDL_LockSurface with SDL_HWPALETTE flag set" + new Error().stack;
  } else {
   HEAPU8.set(surfData.image.data, surfData.buffer);
  }
 }
 return 0;
}

function SDL_unicode() {
 return SDL.unicode;
}

function SDL_ttfContext() {
 return SDL.ttfContext;
}

function SDL_audio() {
 return SDL.audio;
}

var SDL = {
 defaults: {
  width: 320,
  height: 200,
  copyOnLock: true,
  discardOnLock: false,
  opaqueFrontBuffer: true
 },
 version: null,
 surfaces: {},
 canvasPool: [],
 events: [],
 fonts: [ null ],
 audios: [ null ],
 rwops: [ null ],
 music: {
  audio: null,
  volume: 1
 },
 mixerFrequency: 22050,
 mixerFormat: 32784,
 mixerNumChannels: 2,
 mixerChunkSize: 1024,
 channelMinimumNumber: 0,
 GL: false,
 glAttributes: {
  0: 3,
  1: 3,
  2: 2,
  3: 0,
  4: 0,
  5: 1,
  6: 16,
  7: 0,
  8: 0,
  9: 0,
  10: 0,
  11: 0,
  12: 0,
  13: 0,
  14: 0,
  15: 1,
  16: 0,
  17: 0,
  18: 0
 },
 keyboardState: null,
 keyboardMap: {},
 canRequestFullscreen: false,
 isRequestingFullscreen: false,
 textInput: false,
 startTime: null,
 initFlags: 0,
 buttonState: 0,
 modState: 0,
 DOMButtons: [ 0, 0, 0 ],
 DOMEventToSDLEvent: {},
 TOUCH_DEFAULT_ID: 0,
 eventHandler: null,
 eventHandlerContext: null,
 eventHandlerTemp: 0,
 keyCodes: {
  16: 1249,
  17: 1248,
  18: 1250,
  20: 1081,
  33: 1099,
  34: 1102,
  35: 1101,
  36: 1098,
  37: 1104,
  38: 1106,
  39: 1103,
  40: 1105,
  44: 316,
  45: 1097,
  46: 127,
  91: 1251,
  93: 1125,
  96: 1122,
  97: 1113,
  98: 1114,
  99: 1115,
  100: 1116,
  101: 1117,
  102: 1118,
  103: 1119,
  104: 1120,
  105: 1121,
  106: 1109,
  107: 1111,
  109: 1110,
  110: 1123,
  111: 1108,
  112: 1082,
  113: 1083,
  114: 1084,
  115: 1085,
  116: 1086,
  117: 1087,
  118: 1088,
  119: 1089,
  120: 1090,
  121: 1091,
  122: 1092,
  123: 1093,
  124: 1128,
  125: 1129,
  126: 1130,
  127: 1131,
  128: 1132,
  129: 1133,
  130: 1134,
  131: 1135,
  132: 1136,
  133: 1137,
  134: 1138,
  135: 1139,
  144: 1107,
  160: 94,
  161: 33,
  162: 34,
  163: 35,
  164: 36,
  165: 37,
  166: 38,
  167: 95,
  168: 40,
  169: 41,
  170: 42,
  171: 43,
  172: 124,
  173: 45,
  174: 123,
  175: 125,
  176: 126,
  181: 127,
  182: 129,
  183: 128,
  188: 44,
  190: 46,
  191: 47,
  192: 96,
  219: 91,
  220: 92,
  221: 93,
  222: 39,
  224: 1251
 },
 scanCodes: {
  8: 42,
  9: 43,
  13: 40,
  27: 41,
  32: 44,
  35: 204,
  39: 53,
  44: 54,
  46: 55,
  47: 56,
  48: 39,
  49: 30,
  50: 31,
  51: 32,
  52: 33,
  53: 34,
  54: 35,
  55: 36,
  56: 37,
  57: 38,
  58: 203,
  59: 51,
  61: 46,
  91: 47,
  92: 49,
  93: 48,
  96: 52,
  97: 4,
  98: 5,
  99: 6,
  100: 7,
  101: 8,
  102: 9,
  103: 10,
  104: 11,
  105: 12,
  106: 13,
  107: 14,
  108: 15,
  109: 16,
  110: 17,
  111: 18,
  112: 19,
  113: 20,
  114: 21,
  115: 22,
  116: 23,
  117: 24,
  118: 25,
  119: 26,
  120: 27,
  121: 28,
  122: 29,
  127: 76,
  305: 224,
  308: 226,
  316: 70
 },
 loadRect: function(rect) {
  return {
   x: _asan_js_load_4(rect + 0 >> 2),
   y: _asan_js_load_4(rect + 4 >> 2),
   w: _asan_js_load_4(rect + 8 >> 2),
   h: _asan_js_load_4(rect + 12 >> 2)
  };
 },
 updateRect: function(rect, r) {
  _asan_js_store_4(rect >> 2, r.x);
  _asan_js_store_4(rect + 4 >> 2, r.y);
  _asan_js_store_4(rect + 8 >> 2, r.w);
  _asan_js_store_4(rect + 12 >> 2, r.h);
 },
 intersectionOfRects: function(first, second) {
  var leftX = Math.max(first.x, second.x);
  var leftY = Math.max(first.y, second.y);
  var rightX = Math.min(first.x + first.w, second.x + second.w);
  var rightY = Math.min(first.y + first.h, second.y + second.h);
  return {
   x: leftX,
   y: leftY,
   w: Math.max(leftX, rightX) - leftX,
   h: Math.max(leftY, rightY) - leftY
  };
 },
 checkPixelFormat: function(fmt) {
  var format = _asan_js_load_4(fmt >> 2);
  if (format != -2042224636) {
   warnOnce("Unsupported pixel format!");
  }
 },
 loadColorToCSSRGB: function(color) {
  var rgba = _asan_js_load_4(color >> 2);
  return "rgb(" + (rgba & 255) + "," + (rgba >> 8 & 255) + "," + (rgba >> 16 & 255) + ")";
 },
 loadColorToCSSRGBA: function(color) {
  var rgba = _asan_js_load_4(color >> 2);
  return "rgba(" + (rgba & 255) + "," + (rgba >> 8 & 255) + "," + (rgba >> 16 & 255) + "," + (rgba >> 24 & 255) / 255 + ")";
 },
 translateColorToCSSRGBA: function(rgba) {
  return "rgba(" + (rgba & 255) + "," + (rgba >> 8 & 255) + "," + (rgba >> 16 & 255) + "," + (rgba >>> 24) / 255 + ")";
 },
 translateRGBAToCSSRGBA: function(r, g, b, a) {
  return "rgba(" + (r & 255) + "," + (g & 255) + "," + (b & 255) + "," + (a & 255) / 255 + ")";
 },
 translateRGBAToColor: function(r, g, b, a) {
  return r | g << 8 | b << 16 | a << 24;
 },
 makeSurface: function(width, height, flags, usePageCanvas, source, rmask, gmask, bmask, amask) {
  flags = flags || 0;
  var is_SDL_HWSURFACE = flags & 1;
  var is_SDL_HWPALETTE = flags & 2097152;
  var is_SDL_OPENGL = flags & 67108864;
  var surf = _malloc(60);
  var pixelFormat = _malloc(44);
  var bpp = is_SDL_HWPALETTE ? 1 : 4;
  var buffer = 0;
  if (!is_SDL_HWSURFACE && !is_SDL_OPENGL) {
   buffer = _malloc(width * height * 4);
  }
  _asan_js_store_4(surf >> 2, flags);
  _asan_js_store_4(surf + 4 >> 2, pixelFormat);
  _asan_js_store_4(surf + 8 >> 2, width);
  _asan_js_store_4(surf + 12 >> 2, height);
  _asan_js_store_4(surf + 16 >> 2, width * bpp);
  _asan_js_store_4(surf + 20 >> 2, buffer);
  _asan_js_store_4(surf + 36 >> 2, 0);
  _asan_js_store_4(surf + 40 >> 2, 0);
  _asan_js_store_4(surf + 44 >> 2, Module["canvas"].width);
  _asan_js_store_4(surf + 48 >> 2, Module["canvas"].height);
  _asan_js_store_4(surf + 56 >> 2, 1);
  _asan_js_store_4(pixelFormat >> 2, -2042224636);
  _asan_js_store_4(pixelFormat + 4 >> 2, 0);
  _asan_js_store_1(pixelFormat + 8 >> 0, bpp * 8);
  _asan_js_store_1(pixelFormat + 9 >> 0, bpp);
  _asan_js_store_4(pixelFormat + 12 >> 2, rmask || 255);
  _asan_js_store_4(pixelFormat + 16 >> 2, gmask || 65280);
  _asan_js_store_4(pixelFormat + 20 >> 2, bmask || 16711680);
  _asan_js_store_4(pixelFormat + 24 >> 2, amask || 4278190080);
  SDL.GL = SDL.GL || is_SDL_OPENGL;
  var canvas;
  if (!usePageCanvas) {
   if (SDL.canvasPool.length > 0) {
    canvas = SDL.canvasPool.pop();
   } else {
    canvas = document.createElement("canvas");
   }
   canvas.width = width;
   canvas.height = height;
  } else {
   canvas = Module["canvas"];
  }
  var webGLContextAttributes = {
   antialias: SDL.glAttributes[13] != 0 && SDL.glAttributes[14] > 1,
   depth: SDL.glAttributes[6] > 0,
   stencil: SDL.glAttributes[7] > 0,
   alpha: SDL.glAttributes[3] > 0
  };
  var ctx = Browser.createContext(canvas, is_SDL_OPENGL, usePageCanvas, webGLContextAttributes);
  SDL.surfaces[surf] = {
   width: width,
   height: height,
   canvas: canvas,
   ctx: ctx,
   surf: surf,
   buffer: buffer,
   pixelFormat: pixelFormat,
   alpha: 255,
   flags: flags,
   locked: 0,
   usePageCanvas: usePageCanvas,
   source: source,
   isFlagSet: function(flag) {
    return flags & flag;
   }
  };
  return surf;
 },
 copyIndexedColorData: function(surfData, rX, rY, rW, rH) {
  if (!surfData.colors) {
   return;
  }
  var fullWidth = Module["canvas"].width;
  var fullHeight = Module["canvas"].height;
  var startX = rX || 0;
  var startY = rY || 0;
  var endX = (rW || fullWidth - startX) + startX;
  var endY = (rH || fullHeight - startY) + startY;
  var buffer = surfData.buffer;
  if (!surfData.image.data32) {
   surfData.image.data32 = new Uint32Array(surfData.image.data.buffer);
  }
  var data32 = surfData.image.data32;
  var colors32 = surfData.colors32;
  for (var y = startY; y < endY; ++y) {
   var base = y * fullWidth;
   for (var x = startX; x < endX; ++x) {
    data32[base + x] = colors32[_asan_js_load_1u(buffer + base + x >> 0)];
   }
  }
 },
 freeSurface: function(surf) {
  var refcountPointer = surf + 56;
  var refcount = _asan_js_load_4(refcountPointer >> 2);
  if (refcount > 1) {
   _asan_js_store_4(refcountPointer >> 2, refcount - 1);
   return;
  }
  var info = SDL.surfaces[surf];
  if (!info.usePageCanvas && info.canvas) SDL.canvasPool.push(info.canvas);
  if (info.buffer) _free(info.buffer);
  _free(info.pixelFormat);
  _free(surf);
  SDL.surfaces[surf] = null;
  if (surf === SDL.screen) {
   SDL.screen = null;
  }
 },
 blitSurface: function(src, srcrect, dst, dstrect, scale) {
  var srcData = SDL.surfaces[src];
  var dstData = SDL.surfaces[dst];
  var sr, dr;
  if (srcrect) {
   sr = SDL.loadRect(srcrect);
  } else {
   sr = {
    x: 0,
    y: 0,
    w: srcData.width,
    h: srcData.height
   };
  }
  if (dstrect) {
   dr = SDL.loadRect(dstrect);
  } else {
   dr = {
    x: 0,
    y: 0,
    w: srcData.width,
    h: srcData.height
   };
  }
  if (dstData.clipRect) {
   var widthScale = !scale || sr.w === 0 ? 1 : sr.w / dr.w;
   var heightScale = !scale || sr.h === 0 ? 1 : sr.h / dr.h;
   dr = SDL.intersectionOfRects(dstData.clipRect, dr);
   sr.w = dr.w * widthScale;
   sr.h = dr.h * heightScale;
   if (dstrect) {
    SDL.updateRect(dstrect, dr);
   }
  }
  var blitw, blith;
  if (scale) {
   blitw = dr.w;
   blith = dr.h;
  } else {
   blitw = sr.w;
   blith = sr.h;
  }
  if (sr.w === 0 || sr.h === 0 || blitw === 0 || blith === 0) {
   return 0;
  }
  var oldAlpha = dstData.ctx.globalAlpha;
  dstData.ctx.globalAlpha = srcData.alpha / 255;
  dstData.ctx.drawImage(srcData.canvas, sr.x, sr.y, sr.w, sr.h, dr.x, dr.y, blitw, blith);
  dstData.ctx.globalAlpha = oldAlpha;
  if (dst != SDL.screen) {
   warnOnce("WARNING: copying canvas data to memory for compatibility");
   _SDL_LockSurface(dst);
   dstData.locked--;
  }
  return 0;
 },
 downFingers: {},
 savedKeydown: null,
 receiveEvent: function(event) {
  function unpressAllPressedKeys() {
   for (var code in SDL.keyboardMap) {
    SDL.events.push({
     type: "keyup",
     keyCode: SDL.keyboardMap[code]
    });
   }
  }
  switch (event.type) {
  case "touchstart":
  case "touchmove":
   {
    event.preventDefault();
    var touches = [];
    if (event.type === "touchstart") {
     for (var i = 0; i < event.touches.length; i++) {
      var touch = event.touches[i];
      if (SDL.downFingers[touch.identifier] != true) {
       SDL.downFingers[touch.identifier] = true;
       touches.push(touch);
      }
     }
    } else {
     touches = event.touches;
    }
    var firstTouch = touches[0];
    if (firstTouch) {
     if (event.type == "touchstart") {
      SDL.DOMButtons[0] = 1;
     }
     var mouseEventType;
     switch (event.type) {
     case "touchstart":
      mouseEventType = "mousedown";
      break;

     case "touchmove":
      mouseEventType = "mousemove";
      break;
     }
     var mouseEvent = {
      type: mouseEventType,
      button: 0,
      pageX: firstTouch.clientX,
      pageY: firstTouch.clientY
     };
     SDL.events.push(mouseEvent);
    }
    for (var i = 0; i < touches.length; i++) {
     var touch = touches[i];
     SDL.events.push({
      type: event.type,
      touch: touch
     });
    }
    break;
   }

  case "touchend":
   {
    event.preventDefault();
    for (var i = 0; i < event.changedTouches.length; i++) {
     var touch = event.changedTouches[i];
     if (SDL.downFingers[touch.identifier] === true) {
      delete SDL.downFingers[touch.identifier];
     }
    }
    var mouseEvent = {
     type: "mouseup",
     button: 0,
     pageX: event.changedTouches[0].clientX,
     pageY: event.changedTouches[0].clientY
    };
    SDL.DOMButtons[0] = 0;
    SDL.events.push(mouseEvent);
    for (var i = 0; i < event.changedTouches.length; i++) {
     var touch = event.changedTouches[i];
     SDL.events.push({
      type: "touchend",
      touch: touch
     });
    }
    break;
   }

  case "DOMMouseScroll":
  case "mousewheel":
  case "wheel":
   var delta = -Browser.getMouseWheelDelta(event);
   delta = delta == 0 ? 0 : delta > 0 ? Math.max(delta, 1) : Math.min(delta, -1);
   var button = delta > 0 ? 3 : 4;
   SDL.events.push({
    type: "mousedown",
    button: button,
    pageX: event.pageX,
    pageY: event.pageY
   });
   SDL.events.push({
    type: "mouseup",
    button: button,
    pageX: event.pageX,
    pageY: event.pageY
   });
   SDL.events.push({
    type: "wheel",
    deltaX: 0,
    deltaY: delta
   });
   event.preventDefault();
   break;

  case "mousemove":
   if (SDL.DOMButtons[0] === 1) {
    SDL.events.push({
     type: "touchmove",
     touch: {
      identifier: 0,
      deviceID: -1,
      pageX: event.pageX,
      pageY: event.pageY
     }
    });
   }
   if (Browser.pointerLock) {
    if ("mozMovementX" in event) {
     event["movementX"] = event["mozMovementX"];
     event["movementY"] = event["mozMovementY"];
    }
    if (event["movementX"] == 0 && event["movementY"] == 0) {
     event.preventDefault();
     return;
    }
   }

  case "keydown":
  case "keyup":
  case "keypress":
  case "mousedown":
  case "mouseup":
   if (event.type !== "keydown" || !SDL_unicode() && !SDL.textInput || (event.keyCode === 8 || event.keyCode === 9)) {
    event.preventDefault();
   }
   if (event.type == "mousedown") {
    SDL.DOMButtons[event.button] = 1;
    SDL.events.push({
     type: "touchstart",
     touch: {
      identifier: 0,
      deviceID: -1,
      pageX: event.pageX,
      pageY: event.pageY
     }
    });
   } else if (event.type == "mouseup") {
    if (!SDL.DOMButtons[event.button]) {
     return;
    }
    SDL.events.push({
     type: "touchend",
     touch: {
      identifier: 0,
      deviceID: -1,
      pageX: event.pageX,
      pageY: event.pageY
     }
    });
    SDL.DOMButtons[event.button] = 0;
   }
   if (event.type === "keydown" || event.type === "mousedown") {
    SDL.canRequestFullscreen = true;
   } else if (event.type === "keyup" || event.type === "mouseup") {
    if (SDL.isRequestingFullscreen) {
     Module["requestFullscreen"](true, true);
     SDL.isRequestingFullscreen = false;
    }
    SDL.canRequestFullscreen = false;
   }
   if (event.type === "keypress" && SDL.savedKeydown) {
    SDL.savedKeydown.keypressCharCode = event.charCode;
    SDL.savedKeydown = null;
   } else if (event.type === "keydown") {
    SDL.savedKeydown = event;
   }
   if (event.type !== "keypress" || SDL.textInput) {
    SDL.events.push(event);
   }
   break;

  case "mouseout":
   for (var i = 0; i < 3; i++) {
    if (SDL.DOMButtons[i]) {
     SDL.events.push({
      type: "mouseup",
      button: i,
      pageX: event.pageX,
      pageY: event.pageY
     });
     SDL.DOMButtons[i] = 0;
    }
   }
   event.preventDefault();
   break;

  case "focus":
   SDL.events.push(event);
   event.preventDefault();
   break;

  case "blur":
   SDL.events.push(event);
   unpressAllPressedKeys();
   event.preventDefault();
   break;

  case "visibilitychange":
   SDL.events.push({
    type: "visibilitychange",
    visible: !document.hidden
   });
   unpressAllPressedKeys();
   event.preventDefault();
   break;

  case "unload":
   if (Browser.mainLoop.runner) {
    SDL.events.push(event);
    Browser.mainLoop.runner();
   }
   return;

  case "resize":
   SDL.events.push(event);
   if (event.preventDefault) {
    event.preventDefault();
   }
   break;
  }
  if (SDL.events.length >= 1e4) {
   err("SDL event queue full, dropping events");
   SDL.events = SDL.events.slice(0, 1e4);
  }
  SDL.flushEventsToHandler();
  return;
 },
 lookupKeyCodeForEvent: function(event) {
  var code = event.keyCode;
  if (code >= 65 && code <= 90) {
   code += 32;
  } else {
   code = SDL.keyCodes[event.keyCode] || event.keyCode;
   if (event.location === 2 && code >= (224 | 1 << 10) && code <= (227 | 1 << 10)) {
    code += 4;
   }
  }
  return code;
 },
 handleEvent: function(event) {
  if (event.handled) return;
  event.handled = true;
  switch (event.type) {
  case "touchstart":
  case "touchend":
  case "touchmove":
   {
    Browser.calculateMouseEvent(event);
    break;
   }

  case "keydown":
  case "keyup":
   {
    var down = event.type === "keydown";
    var code = SDL.lookupKeyCodeForEvent(event);
    _asan_js_store_1(SDL.keyboardState + code >> 0, down);
    SDL.modState = (_asan_js_load_1(SDL.keyboardState + 1248 >> 0) ? 64 : 0) | (_asan_js_load_1(SDL.keyboardState + 1249 >> 0) ? 1 : 0) | (_asan_js_load_1(SDL.keyboardState + 1250 >> 0) ? 256 : 0) | (_asan_js_load_1(SDL.keyboardState + 1252 >> 0) ? 128 : 0) | (_asan_js_load_1(SDL.keyboardState + 1253 >> 0) ? 2 : 0) | (_asan_js_load_1(SDL.keyboardState + 1254 >> 0) ? 512 : 0);
    if (down) {
     SDL.keyboardMap[code] = event.keyCode;
    } else {
     delete SDL.keyboardMap[code];
    }
    break;
   }

  case "mousedown":
  case "mouseup":
   if (event.type == "mousedown") {
    SDL.buttonState |= 1 << event.button;
   } else if (event.type == "mouseup") {
    SDL.buttonState &= ~(1 << event.button);
   }

  case "mousemove":
   {
    Browser.calculateMouseEvent(event);
    break;
   }
  }
 },
 flushEventsToHandler: function() {
  if (!SDL.eventHandler) return;
  while (SDL.pollEvent(SDL.eventHandlerTemp)) {
   wasmTable.get(SDL.eventHandler)(SDL.eventHandlerContext, SDL.eventHandlerTemp);
  }
 },
 pollEvent: function(ptr) {
  if (SDL.initFlags & 512 && SDL.joystickEventState) {
   SDL.queryJoysticks();
  }
  if (ptr) {
   while (SDL.events.length > 0) {
    if (SDL.makeCEvent(SDL.events.shift(), ptr) !== false) return 1;
   }
   return 0;
  } else {
   return SDL.events.length > 0;
  }
 },
 makeCEvent: function(event, ptr) {
  if (typeof event === "number") {
   _memcpy(ptr, event, 28);
   _free(event);
   return;
  }
  SDL.handleEvent(event);
  switch (event.type) {
  case "keydown":
  case "keyup":
   {
    var down = event.type === "keydown";
    var key = SDL.lookupKeyCodeForEvent(event);
    var scan;
    if (key >= 1024) {
     scan = key - 1024;
    } else {
     scan = SDL.scanCodes[key] || key;
    }
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_1(ptr + 8 >> 0, down ? 1 : 0);
    _asan_js_store_1(ptr + 9 >> 0, 0);
    _asan_js_store_4(ptr + 12 >> 2, scan);
    _asan_js_store_4(ptr + 16 >> 2, key);
    _asan_js_store_2(ptr + 20 >> 1, SDL.modState);
    _asan_js_store_4(ptr + 24 >> 2, event.keypressCharCode || key);
    break;
   }

  case "keypress":
   {
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    var cStr = intArrayFromString(String.fromCharCode(event.charCode));
    for (var i = 0; i < cStr.length; ++i) {
     _asan_js_store_1(ptr + (8 + i) >> 0, cStr[i]);
    }
    break;
   }

  case "mousedown":
  case "mouseup":
  case "mousemove":
   {
    if (event.type != "mousemove") {
     var down = event.type === "mousedown";
     _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
     _asan_js_store_4(ptr + 4 >> 2, 0);
     _asan_js_store_4(ptr + 8 >> 2, 0);
     _asan_js_store_4(ptr + 12 >> 2, 0);
     _asan_js_store_1(ptr + 16 >> 0, event.button + 1);
     _asan_js_store_1(ptr + 17 >> 0, down ? 1 : 0);
     _asan_js_store_4(ptr + 20 >> 2, Browser.mouseX);
     _asan_js_store_4(ptr + 24 >> 2, Browser.mouseY);
    } else {
     _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
     _asan_js_store_4(ptr + 4 >> 2, 0);
     _asan_js_store_4(ptr + 8 >> 2, 0);
     _asan_js_store_4(ptr + 12 >> 2, 0);
     _asan_js_store_4(ptr + 16 >> 2, SDL.buttonState);
     _asan_js_store_4(ptr + 20 >> 2, Browser.mouseX);
     _asan_js_store_4(ptr + 24 >> 2, Browser.mouseY);
     _asan_js_store_4(ptr + 28 >> 2, Browser.mouseMovementX);
     _asan_js_store_4(ptr + 32 >> 2, Browser.mouseMovementY);
    }
    break;
   }

  case "wheel":
   {
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_4(ptr + 16 >> 2, event.deltaX);
    _asan_js_store_4(ptr + 20 >> 2, event.deltaY);
    break;
   }

  case "touchstart":
  case "touchend":
  case "touchmove":
   {
    var touch = event.touch;
    if (!Browser.touches[touch.identifier]) break;
    var w = Module["canvas"].width;
    var h = Module["canvas"].height;
    var x = Browser.touches[touch.identifier].x / w;
    var y = Browser.touches[touch.identifier].y / h;
    var lx = Browser.lastTouches[touch.identifier].x / w;
    var ly = Browser.lastTouches[touch.identifier].y / h;
    var dx = x - lx;
    var dy = y - ly;
    if (touch["deviceID"] === undefined) touch.deviceID = SDL.TOUCH_DEFAULT_ID;
    if (dx === 0 && dy === 0 && event.type === "touchmove") return false;
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_4(ptr + 4 >> 2, _SDL_GetTicks());
    tempI64 = [ touch.deviceID >>> 0, (tempDouble = touch.deviceID, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
    _asan_js_store_4(ptr + 8 >> 2, tempI64[0]), _asan_js_store_4(ptr + 12 >> 2, tempI64[1]);
    tempI64 = [ touch.identifier >>> 0, (tempDouble = touch.identifier, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
    _asan_js_store_4(ptr + 16 >> 2, tempI64[0]), _asan_js_store_4(ptr + 20 >> 2, tempI64[1]);
    _asan_js_store_f(ptr + 24 >> 2, x);
    _asan_js_store_f(ptr + 28 >> 2, y);
    _asan_js_store_f(ptr + 32 >> 2, dx);
    _asan_js_store_f(ptr + 36 >> 2, dy);
    if (touch.force !== undefined) {
     _asan_js_store_f(ptr + 40 >> 2, touch.force);
    } else {
     _asan_js_store_f(ptr + 40 >> 2, event.type == "touchend" ? 0 : 1);
    }
    break;
   }

  case "unload":
   {
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    break;
   }

  case "resize":
   {
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_4(ptr + 4 >> 2, event.w);
    _asan_js_store_4(ptr + 8 >> 2, event.h);
    break;
   }

  case "joystick_button_up":
  case "joystick_button_down":
   {
    var state = event.type === "joystick_button_up" ? 0 : 1;
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_1(ptr + 4 >> 0, event.index);
    _asan_js_store_1(ptr + 5 >> 0, event.button);
    _asan_js_store_1(ptr + 6 >> 0, state);
    break;
   }

  case "joystick_axis_motion":
   {
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_1(ptr + 4 >> 0, event.index);
    _asan_js_store_1(ptr + 5 >> 0, event.axis);
    _asan_js_store_4(ptr + 8 >> 2, SDL.joystickAxisValueConversion(event.value));
    break;
   }

  case "focus":
   {
    var SDL_WINDOWEVENT_FOCUS_GAINED = 12;
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_4(ptr + 4 >> 2, 0);
    _asan_js_store_1(ptr + 8 >> 0, SDL_WINDOWEVENT_FOCUS_GAINED);
    break;
   }

  case "blur":
   {
    var SDL_WINDOWEVENT_FOCUS_LOST = 13;
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_4(ptr + 4 >> 2, 0);
    _asan_js_store_1(ptr + 8 >> 0, SDL_WINDOWEVENT_FOCUS_LOST);
    break;
   }

  case "visibilitychange":
   {
    var SDL_WINDOWEVENT_SHOWN = 1;
    var SDL_WINDOWEVENT_HIDDEN = 2;
    var visibilityEventID = event.visible ? SDL_WINDOWEVENT_SHOWN : SDL_WINDOWEVENT_HIDDEN;
    _asan_js_store_4(ptr >> 2, SDL.DOMEventToSDLEvent[event.type]);
    _asan_js_store_4(ptr + 4 >> 2, 0);
    _asan_js_store_1(ptr + 8 >> 0, visibilityEventID);
    break;
   }

  default:
   throw "Unhandled SDL event: " + event.type;
  }
 },
 makeFontString: function(height, fontName) {
  if (fontName.charAt(0) != "'" && fontName.charAt(0) != '"') {
   fontName = '"' + fontName + '"';
  }
  return height + "px " + fontName + ", serif";
 },
 estimateTextWidth: function(fontData, text) {
  var h = fontData.size;
  var fontString = SDL.makeFontString(h, fontData.name);
  var tempCtx = SDL_ttfContext();
  assert(tempCtx, "TTF_Init must have been called");
  tempCtx.font = fontString;
  var ret = tempCtx.measureText(text).width | 0;
  return ret;
 },
 allocateChannels: function(num) {
  if (SDL.numChannels && SDL.numChannels >= num && num != 0) return;
  SDL.numChannels = num;
  SDL.channels = [];
  for (var i = 0; i < num; i++) {
   SDL.channels[i] = {
    audio: null,
    volume: 1
   };
  }
 },
 setGetVolume: function(info, volume) {
  if (!info) return 0;
  var ret = info.volume * 128;
  if (volume != -1) {
   info.volume = Math.min(Math.max(volume, 0), 128) / 128;
   if (info.audio) {
    try {
     info.audio.volume = info.volume;
     if (info.audio.webAudioGainNode) info.audio.webAudioGainNode["gain"]["value"] = info.volume;
    } catch (e) {
     err("setGetVolume failed to set audio volume: " + e);
    }
   }
  }
  return ret;
 },
 setPannerPosition: function(info, x, y, z) {
  if (!info) return;
  if (info.audio) {
   if (info.audio.webAudioPannerNode) {
    info.audio.webAudioPannerNode["setPosition"](x, y, z);
   }
  }
 },
 playWebAudio: function(audio) {
  if (!audio) return;
  if (audio.webAudioNode) return;
  if (!SDL.webAudioAvailable()) return;
  try {
   var webAudio = audio.resource.webAudio;
   audio.paused = false;
   if (!webAudio.decodedBuffer) {
    if (webAudio.onDecodeComplete === undefined) abort("Cannot play back audio object that was not loaded");
    webAudio.onDecodeComplete.push(function() {
     if (!audio.paused) SDL.playWebAudio(audio);
    });
    return;
   }
   audio.webAudioNode = SDL.audioContext["createBufferSource"]();
   audio.webAudioNode["buffer"] = webAudio.decodedBuffer;
   audio.webAudioNode["loop"] = audio.loop;
   audio.webAudioNode["onended"] = function() {
    audio["onended"]();
   };
   audio.webAudioPannerNode = SDL.audioContext["createPanner"]();
   audio.webAudioPannerNode["setPosition"](0, 0, -.5);
   audio.webAudioPannerNode["panningModel"] = "equalpower";
   audio.webAudioGainNode = SDL.audioContext["createGain"]();
   audio.webAudioGainNode["gain"]["value"] = audio.volume;
   audio.webAudioNode["connect"](audio.webAudioPannerNode);
   audio.webAudioPannerNode["connect"](audio.webAudioGainNode);
   audio.webAudioGainNode["connect"](SDL.audioContext["destination"]);
   audio.webAudioNode["start"](0, audio.currentPosition);
   audio.startTime = SDL.audioContext["currentTime"] - audio.currentPosition;
  } catch (e) {
   err("playWebAudio failed: " + e);
  }
 },
 pauseWebAudio: function(audio) {
  if (!audio) return;
  if (audio.webAudioNode) {
   try {
    audio.currentPosition = (SDL.audioContext["currentTime"] - audio.startTime) % audio.resource.webAudio.decodedBuffer.duration;
    audio.webAudioNode["onended"] = undefined;
    audio.webAudioNode.stop(0);
    audio.webAudioNode = undefined;
   } catch (e) {
    err("pauseWebAudio failed: " + e);
   }
  }
  audio.paused = true;
 },
 openAudioContext: function() {
  if (!SDL.audioContext) {
   if (typeof AudioContext !== "undefined") SDL.audioContext = new AudioContext(); else if (typeof webkitAudioContext !== "undefined") SDL.audioContext = new webkitAudioContext();
  }
 },
 webAudioAvailable: function() {
  return !!SDL.audioContext;
 },
 fillWebAudioBufferFromHeap: function(heapPtr, sizeSamplesPerChannel, dstAudioBuffer) {
  var audio = SDL_audio();
  var numChannels = audio.channels;
  for (var c = 0; c < numChannels; ++c) {
   var channelData = dstAudioBuffer["getChannelData"](c);
   if (channelData.length != sizeSamplesPerChannel) {
    throw "Web Audio output buffer length mismatch! Destination size: " + channelData.length + " samples vs expected " + sizeSamplesPerChannel + " samples!";
   }
   if (audio.format == 32784) {
    for (var j = 0; j < sizeSamplesPerChannel; ++j) {
     channelData[j] = _asan_js_load_2(heapPtr + (j * numChannels + c) * 2 >> 1) / 32768;
    }
   } else if (audio.format == 8) {
    for (var j = 0; j < sizeSamplesPerChannel; ++j) {
     var v = _asan_js_load_1(heapPtr + (j * numChannels + c) >> 0);
     channelData[j] = (v >= 0 ? v - 128 : v + 128) / 128;
    }
   } else if (audio.format == 33056) {
    for (var j = 0; j < sizeSamplesPerChannel; ++j) {
     channelData[j] = _asan_js_load_f(heapPtr + (j * numChannels + c) * 4 >> 2);
    }
   } else {
    throw "Invalid SDL audio format " + audio.format + "!";
   }
  }
 },
 debugSurface: function(surfData) {
  out("dumping surface " + [ surfData.surf, surfData.source, surfData.width, surfData.height ]);
  var image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
  var data = image.data;
  var num = Math.min(surfData.width, surfData.height);
  for (var i = 0; i < num; i++) {
   out("   diagonal " + i + ":" + [ data[i * surfData.width * 4 + i * 4 + 0], data[i * surfData.width * 4 + i * 4 + 1], data[i * surfData.width * 4 + i * 4 + 2], data[i * surfData.width * 4 + i * 4 + 3] ]);
  }
 },
 joystickEventState: 1,
 lastJoystickState: {},
 joystickNamePool: {},
 recordJoystickState: function(joystick, state) {
  var buttons = new Array(state.buttons.length);
  for (var i = 0; i < state.buttons.length; i++) {
   buttons[i] = SDL.getJoystickButtonState(state.buttons[i]);
  }
  SDL.lastJoystickState[joystick] = {
   buttons: buttons,
   axes: state.axes.slice(0),
   timestamp: state.timestamp,
   index: state.index,
   id: state.id
  };
 },
 getJoystickButtonState: function(button) {
  if (typeof button === "object") {
   return button["pressed"];
  } else {
   return button > 0;
  }
 },
 queryJoysticks: function() {
  for (var joystick in SDL.lastJoystickState) {
   var state = SDL.getGamepad(joystick - 1);
   var prevState = SDL.lastJoystickState[joystick];
   if (typeof state === "undefined") return;
   if (state === null) return;
   if (typeof state.timestamp !== "number" || state.timestamp !== prevState.timestamp || !state.timestamp) {
    var i;
    for (i = 0; i < state.buttons.length; i++) {
     var buttonState = SDL.getJoystickButtonState(state.buttons[i]);
     if (buttonState !== prevState.buttons[i]) {
      SDL.events.push({
       type: buttonState ? "joystick_button_down" : "joystick_button_up",
       joystick: joystick,
       index: joystick - 1,
       button: i
      });
     }
    }
    for (i = 0; i < state.axes.length; i++) {
     if (state.axes[i] !== prevState.axes[i]) {
      SDL.events.push({
       type: "joystick_axis_motion",
       joystick: joystick,
       index: joystick - 1,
       axis: i,
       value: state.axes[i]
      });
     }
    }
    SDL.recordJoystickState(joystick, state);
   }
  }
 },
 joystickAxisValueConversion: function(value) {
  value = Math.min(1, Math.max(value, -1));
  return Math.ceil((value + 1) * 32767.5 - 32768);
 },
 getGamepads: function() {
  var fcn = navigator.getGamepads || navigator.webkitGamepads || navigator.mozGamepads || navigator.gamepads || navigator.webkitGetGamepads;
  if (fcn !== undefined) {
   return fcn.apply(navigator);
  } else {
   return [];
  }
 },
 getGamepad: function(deviceIndex) {
  var gamepads = SDL.getGamepads();
  if (gamepads.length > deviceIndex && deviceIndex >= 0) {
   return gamepads[deviceIndex];
  }
  return null;
 }
};

function _Mix_AllocateChannels(num) {
 SDL.allocateChannels(num);
 return num;
}

function _Mix_FreeChunk(id) {
 SDL.audios[id] = null;
}

function _Mix_HaltChannel(channel) {
 function halt(channel) {
  var info = SDL.channels[channel];
  if (info.audio) {
   info.audio.pause();
   info.audio = null;
  }
  if (SDL.channelFinished) {
   wasmTable.get(SDL.channelFinished)(channel);
  }
 }
 if (channel != -1) {
  halt(channel);
 } else {
  for (var i = 0; i < SDL.channels.length; ++i) halt(i);
 }
 return 0;
}

function _Mix_LoadWAV_RW(rwopsID, freesrc) {
 var rwops = SDL.rwops[rwopsID];
 if (rwops === undefined) return 0;
 var filename = "";
 var audio;
 var webAudio;
 var bytes;
 if (rwops.filename !== undefined) {
  filename = PATH_FS.resolve(rwops.filename);
  var raw = Module["preloadedAudios"][filename];
  if (!raw) {
   if (raw === null) err("Trying to reuse preloaded audio, but freePreloadedMediaOnUse is set!");
   if (!Module.noAudioDecoding) warnOnce("Cannot find preloaded audio " + filename);
   try {
    bytes = FS.readFile(filename);
   } catch (e) {
    err("Couldn't find file for: " + filename);
    return 0;
   }
  }
  if (Module["freePreloadedMediaOnUse"]) {
   Module["preloadedAudios"][filename] = null;
  }
  audio = raw;
 } else if (rwops.bytes !== undefined) {
  if (SDL.webAudioAvailable()) bytes = HEAPU8.buffer.slice(rwops.bytes, rwops.bytes + rwops.count); else bytes = HEAPU8.subarray(rwops.bytes, rwops.bytes + rwops.count);
 } else {
  return 0;
 }
 var arrayBuffer = bytes ? bytes.buffer || bytes : bytes;
 var canPlayWithWebAudio = Module["SDL_canPlayWithWebAudio"] === undefined || Module["SDL_canPlayWithWebAudio"](filename, arrayBuffer);
 if (bytes !== undefined && SDL.webAudioAvailable() && canPlayWithWebAudio) {
  audio = undefined;
  webAudio = {};
  webAudio.onDecodeComplete = [];
  var onDecodeComplete = function(data) {
   webAudio.decodedBuffer = data;
   webAudio.onDecodeComplete.forEach(function(e) {
    e();
   });
   webAudio.onDecodeComplete = undefined;
  };
  SDL.audioContext["decodeAudioData"](arrayBuffer, onDecodeComplete);
 } else if (audio === undefined && bytes) {
  var blob = new Blob([ bytes ], {
   type: rwops.mimetype
  });
  var url = URL.createObjectURL(blob);
  audio = new Audio();
  audio.src = url;
  audio.mozAudioChannelType = "content";
 }
 var id = SDL.audios.length;
 SDL.audios.push({
  source: filename,
  audio: audio,
  webAudio: webAudio
 });
 return id;
}

function _SDL_RWFromFile(_name, mode) {
 var id = SDL.rwops.length;
 var name = UTF8ToString(_name);
 SDL.rwops.push({
  filename: name,
  mimetype: Browser.getMimetype(name)
 });
 return id;
}

function _SDL_FreeRW(rwopsID) {
 SDL.rwops[rwopsID] = null;
 while (SDL.rwops.length > 0 && SDL.rwops[SDL.rwops.length - 1] === null) {
  SDL.rwops.pop();
 }
}

function _Mix_LoadWAV(filename) {
 var rwops = _SDL_RWFromFile(filename);
 var result = _Mix_LoadWAV_RW(rwops);
 _SDL_FreeRW(rwops);
 return result;
}

function listenOnce(object, event, func) {
 object.addEventListener(event, func, {
  "once": true
 });
}

function autoResumeAudioContext(ctx, elements) {
 if (!elements) {
  elements = [ document, document.getElementById("canvas") ];
 }
 [ "keydown", "mousedown", "touchstart" ].forEach(function(event) {
  elements.forEach(function(element) {
   if (element) {
    listenOnce(element, event, function() {
     if (ctx.state === "suspended") ctx.resume();
    });
   }
  });
 });
}

function _Mix_OpenAudio(frequency, format, channels, chunksize) {
 SDL.openAudioContext();
 autoResumeAudioContext(SDL.audioContext);
 SDL.allocateChannels(32);
 SDL.mixerFrequency = frequency;
 SDL.mixerFormat = format;
 SDL.mixerNumChannels = channels;
 SDL.mixerChunkSize = chunksize;
 return 0;
}

function _Mix_PlayChannel(channel, id, loops) {
 var info = SDL.audios[id];
 if (!info) return -1;
 if (!info.audio && !info.webAudio) return -1;
 if (channel == -1) {
  for (var i = SDL.channelMinimumNumber; i < SDL.numChannels; i++) {
   if (!SDL.channels[i].audio) {
    channel = i;
    break;
   }
  }
  if (channel == -1) {
   err("All " + SDL.numChannels + " channels in use!");
   return -1;
  }
 }
 var channelInfo = SDL.channels[channel];
 var audio;
 if (info.webAudio) {
  audio = {};
  audio.resource = info;
  audio.paused = false;
  audio.currentPosition = 0;
  audio.play = function() {
   SDL.playWebAudio(this);
  };
  audio.pause = function() {
   SDL.pauseWebAudio(this);
  };
 } else {
  audio = info.audio.cloneNode(true);
  audio.numChannels = info.audio.numChannels;
  audio.frequency = info.audio.frequency;
 }
 audio["onended"] = function SDL_audio_onended() {
  if (channelInfo.audio == this) {
   channelInfo.audio.paused = true;
   channelInfo.audio = null;
  }
  if (SDL.channelFinished) wasmTable.get(SDL.channelFinished)(channel);
 };
 channelInfo.audio = audio;
 audio.loop = loops != 0;
 audio.volume = channelInfo.volume;
 audio.play();
 return channel;
}

function _Mix_PlayChannelTimed(a0, a1, a2) {
 return _Mix_PlayChannel(a0, a1, a2);
}

function _Mix_Playing(channel) {
 if (channel === -1) {
  var count = 0;
  for (var i = 0; i < SDL.channels.length; i++) {
   count += _Mix_Playing(i);
  }
  return count;
 }
 var info = SDL.channels[channel];
 if (info && info.audio && !info.audio.paused) {
  return 1;
 }
 return 0;
}

function _Mix_QuickLoad_RAW(mem, len) {
 var audio;
 var webAudio;
 var numSamples = len >> 1;
 var buffer = new Float32Array(numSamples);
 for (var i = 0; i < numSamples; ++i) {
  buffer[i] = _asan_js_load_2(mem + i * 2 >> 1) / 32768;
 }
 if (SDL.webAudioAvailable()) {
  webAudio = {};
  webAudio.decodedBuffer = buffer;
 } else {
  audio = new Audio();
  audio.mozAudioChannelType = "content";
  audio.numChannels = SDL.mixerNumChannels;
  audio.frequency = SDL.mixerFrequency;
 }
 var id = SDL.audios.length;
 SDL.audios.push({
  source: "",
  audio: audio,
  webAudio: webAudio,
  buffer: buffer
 });
 return id;
}

function _Mix_Volume(channel, volume) {
 if (channel == -1) {
  for (var i = 0; i < SDL.numChannels - 1; i++) {
   _Mix_Volume(i, volume);
  }
  return _Mix_Volume(SDL.numChannels - 1, volume);
 }
 return SDL.setGetVolume(SDL.channels[channel], volume);
}

function _SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask) {
 return SDL.makeSurface(width, height, flags, false, "CreateRGBSurface", rmask, gmask, bmask, amask);
}

function _SDL_Delay(delay) {
 if (!ENVIRONMENT_IS_WORKER) abort("SDL_Delay called on the main thread! Potential infinite loop, quitting. (consider building with async support like ASYNCIFY)");
 var now = Date.now();
 while (Date.now() - now < delay) {}
}

function _SDL_EnableKeyRepeat(delay, interval) {}

function _SDL_EnableUNICODE(on) {
 var ret = SDL.unicode || 0;
 SDL.unicode = on;
 return ret;
}

function _SDL_FreeSurface(surf) {
 if (surf) SDL.freeSurface(surf);
}

function _SDL_GetAppState() {
 var state = 0;
 if (Browser.pointerLock) {
  state |= 1;
 }
 if (document.hasFocus()) {
  state |= 2;
 }
 state |= 4;
 return state;
}

function _SDL_GetError() {
 if (!SDL.errorMessage) {
  SDL.errorMessage = allocate(intArrayFromString("unknown SDL-emscripten error"), ALLOC_NORMAL);
 }
 return SDL.errorMessage;
}

function _SDL_GetKeyboardState(numKeys) {
 if (numKeys) {
  _asan_js_store_4(numKeys >> 2, 65536);
 }
 return SDL.keyboardState;
}

function _SDL_Init(initFlags) {
 SDL.startTime = Date.now();
 SDL.initFlags = initFlags;
 if (!Module["doNotCaptureKeyboard"]) {
  var keyboardListeningElement = Module["keyboardListeningElement"] || document;
  keyboardListeningElement.addEventListener("keydown", SDL.receiveEvent);
  keyboardListeningElement.addEventListener("keyup", SDL.receiveEvent);
  keyboardListeningElement.addEventListener("keypress", SDL.receiveEvent);
  window.addEventListener("focus", SDL.receiveEvent);
  window.addEventListener("blur", SDL.receiveEvent);
  document.addEventListener("visibilitychange", SDL.receiveEvent);
 }
 window.addEventListener("unload", SDL.receiveEvent);
 SDL.keyboardState = _malloc(65536);
 zeroMemory(SDL.keyboardState, 65536);
 SDL.DOMEventToSDLEvent["keydown"] = 768;
 SDL.DOMEventToSDLEvent["keyup"] = 769;
 SDL.DOMEventToSDLEvent["keypress"] = 771;
 SDL.DOMEventToSDLEvent["mousedown"] = 1025;
 SDL.DOMEventToSDLEvent["mouseup"] = 1026;
 SDL.DOMEventToSDLEvent["mousemove"] = 1024;
 SDL.DOMEventToSDLEvent["wheel"] = 1027;
 SDL.DOMEventToSDLEvent["touchstart"] = 1792;
 SDL.DOMEventToSDLEvent["touchend"] = 1793;
 SDL.DOMEventToSDLEvent["touchmove"] = 1794;
 SDL.DOMEventToSDLEvent["unload"] = 256;
 SDL.DOMEventToSDLEvent["resize"] = 28673;
 SDL.DOMEventToSDLEvent["visibilitychange"] = 512;
 SDL.DOMEventToSDLEvent["focus"] = 512;
 SDL.DOMEventToSDLEvent["blur"] = 512;
 SDL.DOMEventToSDLEvent["joystick_axis_motion"] = 1536;
 SDL.DOMEventToSDLEvent["joystick_button_down"] = 1539;
 SDL.DOMEventToSDLEvent["joystick_button_up"] = 1540;
 return 0;
}

function _SDL_ListModes(format, flags) {
 return -1;
}

function _SDL_PollEvent(ptr) {
 return SDL.pollEvent(ptr);
}

function _SDL_AudioQuit() {
 for (var i = 0; i < SDL.numChannels; ++i) {
  if (SDL.channels[i].audio) {
   SDL.channels[i].audio.pause();
   SDL.channels[i].audio = undefined;
  }
 }
 if (SDL.music.audio) SDL.music.audio.pause();
 SDL.music.audio = undefined;
}

function _SDL_Quit() {
 _SDL_AudioQuit();
 out("SDL_Quit called (and ignored)");
}

function __webgl_enable_ANGLE_instanced_arrays(ctx) {
 var ext = ctx.getExtension("ANGLE_instanced_arrays");
 if (ext) {
  ctx["vertexAttribDivisor"] = function(index, divisor) {
   ext["vertexAttribDivisorANGLE"](index, divisor);
  };
  ctx["drawArraysInstanced"] = function(mode, first, count, primcount) {
   ext["drawArraysInstancedANGLE"](mode, first, count, primcount);
  };
  ctx["drawElementsInstanced"] = function(mode, count, type, indices, primcount) {
   ext["drawElementsInstancedANGLE"](mode, count, type, indices, primcount);
  };
  return 1;
 }
}

function __webgl_enable_OES_vertex_array_object(ctx) {
 var ext = ctx.getExtension("OES_vertex_array_object");
 if (ext) {
  ctx["createVertexArray"] = function() {
   return ext["createVertexArrayOES"]();
  };
  ctx["deleteVertexArray"] = function(vao) {
   ext["deleteVertexArrayOES"](vao);
  };
  ctx["bindVertexArray"] = function(vao) {
   ext["bindVertexArrayOES"](vao);
  };
  ctx["isVertexArray"] = function(vao) {
   return ext["isVertexArrayOES"](vao);
  };
  return 1;
 }
}

function __webgl_enable_WEBGL_draw_buffers(ctx) {
 var ext = ctx.getExtension("WEBGL_draw_buffers");
 if (ext) {
  ctx["drawBuffers"] = function(n, bufs) {
   ext["drawBuffersWEBGL"](n, bufs);
  };
  return 1;
 }
}

function __webgl_enable_WEBGL_multi_draw(ctx) {
 return !!(ctx.multiDrawWebgl = ctx.getExtension("WEBGL_multi_draw"));
}

var GL = {
 counter: 1,
 buffers: [],
 programs: [],
 framebuffers: [],
 renderbuffers: [],
 textures: [],
 shaders: [],
 vaos: [],
 contexts: [],
 offscreenCanvases: {},
 queries: [],
 stringCache: {},
 unpackAlignment: 4,
 recordError: function recordError(errorCode) {
  if (!GL.lastError) {
   GL.lastError = errorCode;
  }
 },
 getNewId: function(table) {
  var ret = GL.counter++;
  for (var i = table.length; i < ret; i++) {
   table[i] = null;
  }
  return ret;
 },
 getSource: function(shader, count, string, length) {
  var source = "";
  for (var i = 0; i < count; ++i) {
   var len = length ? _asan_js_load_4(length + i * 4 >> 2) : -1;
   source += UTF8ToString(_asan_js_load_4(string + i * 4 >> 2), len < 0 ? undefined : len);
  }
  return source;
 },
 createContext: function(canvas, webGLContextAttributes) {
  if (!canvas.getContextSafariWebGL2Fixed) {
   canvas.getContextSafariWebGL2Fixed = canvas.getContext;
   canvas.getContext = function(ver, attrs) {
    var gl = canvas.getContextSafariWebGL2Fixed(ver, attrs);
    return ver == "webgl" == gl instanceof WebGLRenderingContext ? gl : null;
   };
  }
  var ctx = canvas.getContext("webgl", webGLContextAttributes);
  if (!ctx) return 0;
  var handle = GL.registerContext(ctx, webGLContextAttributes);
  return handle;
 },
 registerContext: function(ctx, webGLContextAttributes) {
  var handle = GL.getNewId(GL.contexts);
  var context = {
   handle: handle,
   attributes: webGLContextAttributes,
   version: webGLContextAttributes.majorVersion,
   GLctx: ctx
  };
  if (ctx.canvas) ctx.canvas.GLctxObject = context;
  GL.contexts[handle] = context;
  if (typeof webGLContextAttributes.enableExtensionsByDefault === "undefined" || webGLContextAttributes.enableExtensionsByDefault) {
   GL.initExtensions(context);
  }
  return handle;
 },
 makeContextCurrent: function(contextHandle) {
  GL.currentContext = GL.contexts[contextHandle];
  Module.ctx = GLctx = GL.currentContext && GL.currentContext.GLctx;
  return !(contextHandle && !GLctx);
 },
 getContext: function(contextHandle) {
  return GL.contexts[contextHandle];
 },
 deleteContext: function(contextHandle) {
  if (GL.currentContext === GL.contexts[contextHandle]) GL.currentContext = null;
  if (typeof JSEvents === "object") JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas);
  if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined;
  GL.contexts[contextHandle] = null;
 },
 initExtensions: function(context) {
  if (!context) context = GL.currentContext;
  if (context.initExtensionsDone) return;
  context.initExtensionsDone = true;
  var GLctx = context.GLctx;
  __webgl_enable_ANGLE_instanced_arrays(GLctx);
  __webgl_enable_OES_vertex_array_object(GLctx);
  __webgl_enable_WEBGL_draw_buffers(GLctx);
  {
   GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
  }
  __webgl_enable_WEBGL_multi_draw(GLctx);
  var exts = GLctx.getSupportedExtensions() || [];
  exts.forEach(function(ext) {
   if (!ext.includes("lose_context") && !ext.includes("debug")) {
    GLctx.getExtension(ext);
   }
  });
 }
};

function _SDL_SetVideoMode(width, height, depth, flags) {
 [ "touchstart", "touchend", "touchmove", "mousedown", "mouseup", "mousemove", "DOMMouseScroll", "mousewheel", "wheel", "mouseout" ].forEach(function(event) {
  Module["canvas"].addEventListener(event, SDL.receiveEvent, true);
 });
 var canvas = Module["canvas"];
 if (width == 0 && height == 0) {
  width = canvas.width;
  height = canvas.height;
 }
 if (!SDL.addedResizeListener) {
  SDL.addedResizeListener = true;
  Browser.resizeListeners.push(function(w, h) {
   if (!SDL.settingVideoMode) {
    SDL.receiveEvent({
     type: "resize",
     w: w,
     h: h
    });
   }
  });
 }
 SDL.settingVideoMode = true;
 Browser.setCanvasSize(width, height);
 SDL.settingVideoMode = false;
 if (SDL.screen) {
  SDL.freeSurface(SDL.screen);
  assert(!SDL.screen);
 }
 if (SDL.GL) flags = flags | 67108864;
 SDL.screen = SDL.makeSurface(width, height, flags, true, "screen");
 return SDL.screen;
}

function _SDL_ShowCursor(toggle) {
 switch (toggle) {
 case 0:
  if (Browser.isFullscreen) {
   Module["canvas"].requestPointerLock();
   return 0;
  } else {
   return 1;
  }
  break;

 case 1:
  Module["canvas"].exitPointerLock();
  return 1;
  break;

 case -1:
  return !Browser.pointerLock;
  break;

 default:
  out("SDL_ShowCursor called with unknown toggle parameter value: " + toggle + ".");
  break;
 }
}

function _SDL_UnlockSurface(surf) {
 assert(!SDL.GL);
 var surfData = SDL.surfaces[surf];
 if (!surfData.locked || --surfData.locked > 0) {
  return;
 }
 if (surfData.isFlagSet(2097152)) {
  SDL.copyIndexedColorData(surfData);
 } else if (!surfData.colors) {
  var data = surfData.image.data;
  var buffer = surfData.buffer;
  assert(buffer % 4 == 0, "Invalid buffer offset: " + buffer);
  var src = buffer >> 2;
  var dst = 0;
  var isScreen = surf == SDL.screen;
  var num;
  if (typeof CanvasPixelArray !== "undefined" && data instanceof CanvasPixelArray) {
   num = data.length;
   while (dst < num) {
    var val = _asan_js_load_4(src);
    data[dst] = val & 255;
    data[dst + 1] = val >> 8 & 255;
    data[dst + 2] = val >> 16 & 255;
    data[dst + 3] = isScreen ? 255 : val >> 24 & 255;
    src++;
    dst += 4;
   }
  } else {
   var data32 = new Uint32Array(data.buffer);
   if (isScreen && SDL.defaults.opaqueFrontBuffer) {
    num = data32.length;
    data32.set(HEAP32.subarray(src, src + num));
    var data8 = new Uint8Array(data.buffer);
    var i = 3;
    var j = i + 4 * num;
    if (num % 8 == 0) {
     while (i < j) {
      data8[i] = 255;
      i = i + 4 | 0;
      data8[i] = 255;
      i = i + 4 | 0;
      data8[i] = 255;
      i = i + 4 | 0;
      data8[i] = 255;
      i = i + 4 | 0;
      data8[i] = 255;
      i = i + 4 | 0;
      data8[i] = 255;
      i = i + 4 | 0;
      data8[i] = 255;
      i = i + 4 | 0;
      data8[i] = 255;
      i = i + 4 | 0;
     }
    } else {
     while (i < j) {
      data8[i] = 255;
      i = i + 4 | 0;
     }
    }
   } else {
    data32.set(HEAP32.subarray(src, src + data32.length));
   }
  }
 } else {
  var width = Module["canvas"].width;
  var height = Module["canvas"].height;
  var s = surfData.buffer;
  var data = surfData.image.data;
  var colors = surfData.colors;
  for (var y = 0; y < height; y++) {
   var base = y * width * 4;
   for (var x = 0; x < width; x++) {
    var val = _asan_js_load_1u(s++ >> 0) * 4;
    var start = base + x * 4;
    data[start] = colors[val];
    data[start + 1] = colors[val + 1];
    data[start + 2] = colors[val + 2];
   }
   s += width * 3;
  }
 }
 surfData.ctx.putImageData(surfData.image, 0, 0);
}

function _SDL_UpdateRect(surf, x, y, w, h) {}

function _SDL_UpperBlit(src, srcrect, dst, dstrect) {
 return SDL.blitSurface(src, srcrect, dst, dstrect, false);
}

function _SDL_WM_GrabInput() {}

function _SDL_WM_SetCaption(title, icon) {
 if (title && typeof setWindowTitle !== "undefined") {
  setWindowTitle(UTF8ToString(title));
 }
 icon = icon && UTF8ToString(icon);
}

function _SDL_WarpMouse(x, y) {
 return;
}

function _SDL_mutexP() {
 return 0;
}

function _SDL_mutexV() {
 return 0;
}

function ___assert_fail(condition, filename, line, func) {
 abort("Assertion failed: " + UTF8ToString(condition) + ", at: " + [ filename ? UTF8ToString(filename) : "unknown filename", line, func ? UTF8ToString(func) : "unknown function" ]);
}

function ___call_sighandler(fp, sig) {
 wasmTable.get(fp)(sig);
}

function ___cxa_allocate_exception(size) {
 return _malloc(size + 16) + 16;
}

function _atexit(func, arg) {
 __ATEXIT__.unshift({
  func: func,
  arg: arg
 });
}

function ___cxa_atexit(a0, a1) {
 return _atexit(a0, a1);
}

function ExceptionInfo(excPtr) {
 this.excPtr = excPtr;
 this.ptr = excPtr - 16;
 this.set_type = function(type) {
  _asan_js_store_4(this.ptr + 4 >> 2, type);
 };
 this.get_type = function() {
  return _asan_js_load_4(this.ptr + 4 >> 2);
 };
 this.set_destructor = function(destructor) {
  _asan_js_store_4(this.ptr + 8 >> 2, destructor);
 };
 this.get_destructor = function() {
  return _asan_js_load_4(this.ptr + 8 >> 2);
 };
 this.set_refcount = function(refcount) {
  _asan_js_store_4(this.ptr >> 2, refcount);
 };
 this.set_caught = function(caught) {
  caught = caught ? 1 : 0;
  _asan_js_store_1(this.ptr + 12 >> 0, caught);
 };
 this.get_caught = function() {
  return _asan_js_load_1(this.ptr + 12 >> 0) != 0;
 };
 this.set_rethrown = function(rethrown) {
  rethrown = rethrown ? 1 : 0;
  _asan_js_store_1(this.ptr + 13 >> 0, rethrown);
 };
 this.get_rethrown = function() {
  return _asan_js_load_1(this.ptr + 13 >> 0) != 0;
 };
 this.init = function(type, destructor) {
  this.set_type(type);
  this.set_destructor(destructor);
  this.set_refcount(0);
  this.set_caught(false);
  this.set_rethrown(false);
 };
 this.add_ref = function() {
  var value = _asan_js_load_4(this.ptr >> 2);
  _asan_js_store_4(this.ptr >> 2, value + 1);
 };
 this.release_ref = function() {
  var prev = _asan_js_load_4(this.ptr >> 2);
  _asan_js_store_4(this.ptr >> 2, prev - 1);
  assert(prev > 0);
  return prev === 1;
 };
}

function CatchInfo(ptr) {
 this.free = function() {
  _free(this.ptr);
  this.ptr = 0;
 };
 this.set_base_ptr = function(basePtr) {
  _asan_js_store_4(this.ptr >> 2, basePtr);
 };
 this.get_base_ptr = function() {
  return _asan_js_load_4(this.ptr >> 2);
 };
 this.set_adjusted_ptr = function(adjustedPtr) {
  _asan_js_store_4(this.ptr + 4 >> 2, adjustedPtr);
 };
 this.get_adjusted_ptr_addr = function() {
  return this.ptr + 4;
 };
 this.get_adjusted_ptr = function() {
  return _asan_js_load_4(this.ptr + 4 >> 2);
 };
 this.get_exception_ptr = function() {
  var isPointer = ___cxa_is_pointer_type(this.get_exception_info().get_type());
  if (isPointer) {
   return _asan_js_load_4(this.get_base_ptr() >> 2);
  }
  var adjusted = this.get_adjusted_ptr();
  if (adjusted !== 0) return adjusted;
  return this.get_base_ptr();
 };
 this.get_exception_info = function() {
  return new ExceptionInfo(this.get_base_ptr());
 };
 if (ptr === undefined) {
  this.ptr = _malloc(8);
  this.set_adjusted_ptr(0);
 } else {
  this.ptr = ptr;
 }
}

var exceptionCaught = [];

function exception_addRef(info) {
 info.add_ref();
}

var uncaughtExceptionCount = 0;

function ___cxa_begin_catch(ptr) {
 var catchInfo = new CatchInfo(ptr);
 var info = catchInfo.get_exception_info();
 if (!info.get_caught()) {
  info.set_caught(true);
  uncaughtExceptionCount--;
 }
 info.set_rethrown(false);
 exceptionCaught.push(catchInfo);
 exception_addRef(info);
 return catchInfo.get_exception_ptr();
}

var exceptionLast = 0;

function ___cxa_free_exception(ptr) {
 try {
  return _free(new ExceptionInfo(ptr).ptr);
 } catch (e) {
  err("exception during cxa_free_exception: " + e);
 }
}

function exception_decRef(info) {
 if (info.release_ref() && !info.get_rethrown()) {
  var destructor = info.get_destructor();
  if (destructor) {
   wasmTable.get(destructor)(info.excPtr);
  }
  ___cxa_free_exception(info.excPtr);
 }
}

function ___cxa_end_catch() {
 _setThrew(0);
 assert(exceptionCaught.length > 0);
 var catchInfo = exceptionCaught.pop();
 exception_decRef(catchInfo.get_exception_info());
 catchInfo.free();
 exceptionLast = 0;
}

function ___resumeException(catchInfoPtr) {
 var catchInfo = new CatchInfo(catchInfoPtr);
 var ptr = catchInfo.get_base_ptr();
 if (!exceptionLast) {
  exceptionLast = ptr;
 }
 catchInfo.free();
 throw ptr;
}

function ___cxa_find_matching_catch_2() {
 var thrown = exceptionLast;
 if (!thrown) {
  setTempRet0(0);
  return 0 | 0;
 }
 var info = new ExceptionInfo(thrown);
 var thrownType = info.get_type();
 var catchInfo = new CatchInfo();
 catchInfo.set_base_ptr(thrown);
 catchInfo.set_adjusted_ptr(thrown);
 if (!thrownType) {
  setTempRet0(0);
  return catchInfo.ptr | 0;
 }
 var typeArray = Array.prototype.slice.call(arguments);
 for (var i = 0; i < typeArray.length; i++) {
  var caughtType = typeArray[i];
  if (caughtType === 0 || caughtType === thrownType) {
   break;
  }
  if (___cxa_can_catch(caughtType, thrownType, catchInfo.get_adjusted_ptr_addr())) {
   setTempRet0(caughtType);
   return catchInfo.ptr | 0;
  }
 }
 setTempRet0(thrownType);
 return catchInfo.ptr | 0;
}

function ___cxa_find_matching_catch_3() {
 var thrown = exceptionLast;
 if (!thrown) {
  setTempRet0(0);
  return 0 | 0;
 }
 var info = new ExceptionInfo(thrown);
 var thrownType = info.get_type();
 var catchInfo = new CatchInfo();
 catchInfo.set_base_ptr(thrown);
 catchInfo.set_adjusted_ptr(thrown);
 if (!thrownType) {
  setTempRet0(0);
  return catchInfo.ptr | 0;
 }
 var typeArray = Array.prototype.slice.call(arguments);
 for (var i = 0; i < typeArray.length; i++) {
  var caughtType = typeArray[i];
  if (caughtType === 0 || caughtType === thrownType) {
   break;
  }
  if (___cxa_can_catch(caughtType, thrownType, catchInfo.get_adjusted_ptr_addr())) {
   setTempRet0(caughtType);
   return catchInfo.ptr | 0;
  }
 }
 setTempRet0(thrownType);
 return catchInfo.ptr | 0;
}

function ___cxa_rethrow() {
 var catchInfo = exceptionCaught.pop();
 if (!catchInfo) {
  abort("no exception to throw");
 }
 var info = catchInfo.get_exception_info();
 var ptr = catchInfo.get_base_ptr();
 if (!info.get_rethrown()) {
  exceptionCaught.push(catchInfo);
  info.set_rethrown(true);
  info.set_caught(false);
  uncaughtExceptionCount++;
 } else {
  catchInfo.free();
 }
 exceptionLast = ptr;
 throw ptr;
}

function ___cxa_throw(ptr, type, destructor) {
 var info = new ExceptionInfo(ptr);
 info.init(type, destructor);
 exceptionLast = ptr;
 uncaughtExceptionCount++;
 throw ptr;
}

function ___cxa_uncaught_exceptions() {
 return uncaughtExceptionCount;
}

function _tzset_impl() {
 var currentYear = new Date().getFullYear();
 var winter = new Date(currentYear, 0, 1);
 var summer = new Date(currentYear, 6, 1);
 var winterOffset = winter.getTimezoneOffset();
 var summerOffset = summer.getTimezoneOffset();
 var stdTimezoneOffset = Math.max(winterOffset, summerOffset);
 _asan_js_store_4(__get_timezone() >> 2, stdTimezoneOffset * 60);
 _asan_js_store_4(__get_daylight() >> 2, Number(winterOffset != summerOffset));
 function extractZone(date) {
  var match = date.toTimeString().match(/\(([A-Za-z ]+)\)$/);
  return match ? match[1] : "GMT";
 }
 var winterName = extractZone(winter);
 var summerName = extractZone(summer);
 var winterNamePtr = allocateUTF8(winterName);
 var summerNamePtr = allocateUTF8(summerName);
 if (summerOffset < winterOffset) {
  _asan_js_store_4(__get_tzname() >> 2, winterNamePtr);
  _asan_js_store_4(__get_tzname() + 4 >> 2, summerNamePtr);
 } else {
  _asan_js_store_4(__get_tzname() >> 2, summerNamePtr);
  _asan_js_store_4(__get_tzname() + 4 >> 2, winterNamePtr);
 }
}

function _tzset() {
 if (_tzset.called) return;
 _tzset.called = true;
 _tzset_impl();
}

function _localtime_r(time, tmPtr) {
 _tzset();
 var date = new Date(_asan_js_load_4(time >> 2) * 1e3);
 _asan_js_store_4(tmPtr >> 2, date.getSeconds());
 _asan_js_store_4(tmPtr + 4 >> 2, date.getMinutes());
 _asan_js_store_4(tmPtr + 8 >> 2, date.getHours());
 _asan_js_store_4(tmPtr + 12 >> 2, date.getDate());
 _asan_js_store_4(tmPtr + 16 >> 2, date.getMonth());
 _asan_js_store_4(tmPtr + 20 >> 2, date.getFullYear() - 1900);
 _asan_js_store_4(tmPtr + 24 >> 2, date.getDay());
 var start = new Date(date.getFullYear(), 0, 1);
 var yday = (date.getTime() - start.getTime()) / (1e3 * 60 * 60 * 24) | 0;
 _asan_js_store_4(tmPtr + 28 >> 2, yday);
 _asan_js_store_4(tmPtr + 36 >> 2, -(date.getTimezoneOffset() * 60));
 var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
 var winterOffset = start.getTimezoneOffset();
 var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset)) | 0;
 _asan_js_store_4(tmPtr + 32 >> 2, dst);
 var zonePtr = _asan_js_load_4(__get_tzname() + (dst ? 4 : 0) >> 2);
 _asan_js_store_4(tmPtr + 40 >> 2, zonePtr);
 return tmPtr;
}

function ___localtime_r(a0, a1) {
 return _localtime_r(a0, a1);
}

var SYSCALLS = {
 mappings: {},
 DEFAULT_POLLMASK: 5,
 umask: 511,
 calculateAt: function(dirfd, path, allowEmpty) {
  if (path[0] === "/") {
   return path;
  }
  var dir;
  if (dirfd === -100) {
   dir = FS.cwd();
  } else {
   var dirstream = FS.getStream(dirfd);
   if (!dirstream) throw new FS.ErrnoError(8);
   dir = dirstream.path;
  }
  if (path.length == 0) {
   if (!allowEmpty) {
    throw new FS.ErrnoError(44);
   }
   return dir;
  }
  return PATH.join2(dir, path);
 },
 doStat: function(func, path, buf) {
  try {
   var stat = func(path);
  } catch (e) {
   if (e && e.node && PATH.normalize(path) !== PATH.normalize(FS.getPath(e.node))) {
    return -54;
   }
   throw e;
  }
  _asan_js_store_4(buf >> 2, stat.dev);
  _asan_js_store_4(buf + 4 >> 2, 0);
  _asan_js_store_4(buf + 8 >> 2, stat.ino);
  _asan_js_store_4(buf + 12 >> 2, stat.mode);
  _asan_js_store_4(buf + 16 >> 2, stat.nlink);
  _asan_js_store_4(buf + 20 >> 2, stat.uid);
  _asan_js_store_4(buf + 24 >> 2, stat.gid);
  _asan_js_store_4(buf + 28 >> 2, stat.rdev);
  _asan_js_store_4(buf + 32 >> 2, 0);
  tempI64 = [ stat.size >>> 0, (tempDouble = stat.size, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
  _asan_js_store_4(buf + 40 >> 2, tempI64[0]), _asan_js_store_4(buf + 44 >> 2, tempI64[1]);
  _asan_js_store_4(buf + 48 >> 2, 4096);
  _asan_js_store_4(buf + 52 >> 2, stat.blocks);
  _asan_js_store_4(buf + 56 >> 2, stat.atime.getTime() / 1e3 | 0);
  _asan_js_store_4(buf + 60 >> 2, 0);
  _asan_js_store_4(buf + 64 >> 2, stat.mtime.getTime() / 1e3 | 0);
  _asan_js_store_4(buf + 68 >> 2, 0);
  _asan_js_store_4(buf + 72 >> 2, stat.ctime.getTime() / 1e3 | 0);
  _asan_js_store_4(buf + 76 >> 2, 0);
  tempI64 = [ stat.ino >>> 0, (tempDouble = stat.ino, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
  _asan_js_store_4(buf + 80 >> 2, tempI64[0]), _asan_js_store_4(buf + 84 >> 2, tempI64[1]);
  return 0;
 },
 doMsync: function(addr, stream, len, flags, offset) {
  var buffer = HEAPU8.slice(addr, addr + len);
  FS.msync(stream, buffer, offset, len, flags);
 },
 doMkdir: function(path, mode) {
  path = PATH.normalize(path);
  if (path[path.length - 1] === "/") path = path.substr(0, path.length - 1);
  FS.mkdir(path, mode, 0);
  return 0;
 },
 doMknod: function(path, mode, dev) {
  switch (mode & 61440) {
  case 32768:
  case 8192:
  case 24576:
  case 4096:
  case 49152:
   break;

  default:
   return -28;
  }
  FS.mknod(path, mode, dev);
  return 0;
 },
 doReadlink: function(path, buf, bufsize) {
  if (bufsize <= 0) return -28;
  var ret = FS.readlink(path);
  var len = Math.min(bufsize, lengthBytesUTF8(ret));
  var endChar = _asan_js_load_1(buf + len);
  stringToUTF8(ret, buf, bufsize + 1);
  _asan_js_store_1(buf + len, endChar);
  return len;
 },
 doAccess: function(path, amode) {
  if (amode & ~7) {
   return -28;
  }
  var node;
  var lookup = FS.lookupPath(path, {
   follow: true
  });
  node = lookup.node;
  if (!node) {
   return -44;
  }
  var perms = "";
  if (amode & 4) perms += "r";
  if (amode & 2) perms += "w";
  if (amode & 1) perms += "x";
  if (perms && FS.nodePermissions(node, perms)) {
   return -2;
  }
  return 0;
 },
 doDup: function(path, flags, suggestFD) {
  var suggest = FS.getStream(suggestFD);
  if (suggest) FS.close(suggest);
  return FS.open(path, flags, 0, suggestFD, suggestFD).fd;
 },
 doReadv: function(stream, iov, iovcnt, offset) {
  var ret = 0;
  for (var i = 0; i < iovcnt; i++) {
   var ptr = _asan_js_load_4(iov + i * 8 >> 2);
   var len = _asan_js_load_4(iov + (i * 8 + 4) >> 2);
   var curr = FS.read(stream, HEAP8, ptr, len, offset);
   if (curr < 0) return -1;
   ret += curr;
   if (curr < len) break;
  }
  return ret;
 },
 doWritev: function(stream, iov, iovcnt, offset) {
  var ret = 0;
  for (var i = 0; i < iovcnt; i++) {
   var ptr = _asan_js_load_4(iov + i * 8 >> 2);
   var len = _asan_js_load_4(iov + (i * 8 + 4) >> 2);
   var curr = FS.write(stream, HEAP8, ptr, len, offset);
   if (curr < 0) return -1;
   ret += curr;
  }
  return ret;
 },
 varargs: undefined,
 get: function() {
  assert(SYSCALLS.varargs != undefined);
  SYSCALLS.varargs += 4;
  var ret = _asan_js_load_4(SYSCALLS.varargs - 4 >> 2);
  return ret;
 },
 getStr: function(ptr) {
  var ret = UTF8ToString(ptr);
  return ret;
 },
 getStreamFromFD: function(fd) {
  var stream = FS.getStream(fd);
  if (!stream) throw new FS.ErrnoError(8);
  return stream;
 },
 get64: function(low, high) {
  if (low >= 0) assert(high === 0); else assert(high === -1);
  return low;
 }
};

function ___sys_chdir(path) {
 try {
  path = SYSCALLS.getStr(path);
  FS.chdir(path);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_dup(fd) {
 try {
  var old = SYSCALLS.getStreamFromFD(fd);
  return FS.open(old.path, old.flags, 0).fd;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function setErrNo(value) {
 _asan_js_store_4(___errno_location() >> 2, value);
 return value;
}

function ___sys_fcntl64(fd, cmd, varargs) {
 SYSCALLS.varargs = varargs;
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  switch (cmd) {
  case 0:
   {
    var arg = SYSCALLS.get();
    if (arg < 0) {
     return -28;
    }
    var newStream;
    newStream = FS.open(stream.path, stream.flags, 0, arg);
    return newStream.fd;
   }

  case 1:
  case 2:
   return 0;

  case 3:
   return stream.flags;

  case 4:
   {
    var arg = SYSCALLS.get();
    stream.flags |= arg;
    return 0;
   }

  case 12:
   {
    var arg = SYSCALLS.get();
    var offset = 0;
    _asan_js_store_2(arg + offset >> 1, 2);
    return 0;
   }

  case 13:
  case 14:
   return 0;

  case 16:
  case 8:
   return -28;

  case 9:
   setErrNo(28);
   return -1;

  default:
   {
    return -28;
   }
  }
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_fstat64(fd, buf) {
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  return SYSCALLS.doStat(FS.stat, stream.path, buf);
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_ftruncate64(fd, zero, low, high) {
 try {
  var length = SYSCALLS.get64(low, high);
  FS.ftruncate(fd, length);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_getcwd(buf, size) {
 try {
  if (size === 0) return -28;
  var cwd = FS.cwd();
  var cwdLengthInBytes = lengthBytesUTF8(cwd);
  if (size < cwdLengthInBytes + 1) return -68;
  stringToUTF8(cwd, buf, size);
  return buf;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_getdents64(fd, dirp, count) {
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  if (!stream.getdents) {
   stream.getdents = FS.readdir(stream.path);
  }
  var struct_size = 280;
  var pos = 0;
  var off = FS.llseek(stream, 0, 1);
  var idx = Math.floor(off / struct_size);
  while (idx < stream.getdents.length && pos + struct_size <= count) {
   var id;
   var type;
   var name = stream.getdents[idx];
   if (name[0] === ".") {
    id = 1;
    type = 4;
   } else {
    var child = FS.lookupNode(stream.node, name);
    id = child.id;
    type = FS.isChrdev(child.mode) ? 2 : FS.isDir(child.mode) ? 4 : FS.isLink(child.mode) ? 10 : 8;
   }
   tempI64 = [ id >>> 0, (tempDouble = id, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
   _asan_js_store_4(dirp + pos >> 2, tempI64[0]), _asan_js_store_4(dirp + pos + 4 >> 2, tempI64[1]);
   tempI64 = [ (idx + 1) * struct_size >>> 0, (tempDouble = (idx + 1) * struct_size, 
   +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
   _asan_js_store_4(dirp + pos + 8 >> 2, tempI64[0]), _asan_js_store_4(dirp + pos + 12 >> 2, tempI64[1]);
   _asan_js_store_2(dirp + pos + 16 >> 1, 280);
   _asan_js_store_1(dirp + pos + 18 >> 0, type);
   stringToUTF8(name, dirp + pos + 19, 256);
   pos += struct_size;
   idx += 1;
  }
  FS.llseek(stream, idx * struct_size, 0);
  return pos;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_getpid() {
 return 42;
}

function ___sys_ioctl(fd, op, varargs) {
 SYSCALLS.varargs = varargs;
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  switch (op) {
  case 21509:
  case 21505:
   {
    if (!stream.tty) return -59;
    return 0;
   }

  case 21510:
  case 21511:
  case 21512:
  case 21506:
  case 21507:
  case 21508:
   {
    if (!stream.tty) return -59;
    return 0;
   }

  case 21519:
   {
    if (!stream.tty) return -59;
    var argp = SYSCALLS.get();
    _asan_js_store_4(argp >> 2, 0);
    return 0;
   }

  case 21520:
   {
    if (!stream.tty) return -59;
    return -28;
   }

  case 21531:
   {
    var argp = SYSCALLS.get();
    return FS.ioctl(stream, op, argp);
   }

  case 21523:
   {
    if (!stream.tty) return -59;
    return 0;
   }

  case 21524:
   {
    if (!stream.tty) return -59;
    return 0;
   }

  default:
   abort("bad ioctl syscall " + op);
  }
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_mkdir(path, mode) {
 try {
  path = SYSCALLS.getStr(path);
  return SYSCALLS.doMkdir(path, mode);
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function syscallMmap2(addr, len, prot, flags, fd, off) {
 off <<= 12;
 var ptr;
 var allocated = false;
 if ((flags & 16) !== 0 && addr % 65536 !== 0) {
  return -28;
 }
 if ((flags & 32) !== 0) {
  ptr = mmapAlloc(len);
  if (!ptr) return -48;
  allocated = true;
 } else {
  var info = FS.getStream(fd);
  if (!info) return -8;
  var res = FS.mmap(info, addr, len, off, prot, flags);
  ptr = res.ptr;
  allocated = res.allocated;
 }
 SYSCALLS.mappings[ptr] = {
  malloc: ptr,
  len: len,
  allocated: allocated,
  fd: fd,
  prot: prot,
  flags: flags,
  offset: off
 };
 return ptr;
}

function ___sys_mmap2(addr, len, prot, flags, fd, off) {
 try {
  return syscallMmap2(addr, len, prot, flags, fd, off);
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function syscallMunmap(addr, len) {
 var info = SYSCALLS.mappings[addr];
 if (len === 0 || !info) {
  return -28;
 }
 if (len === info.len) {
  var stream = FS.getStream(info.fd);
  if (stream) {
   if (info.prot & 2) {
    SYSCALLS.doMsync(addr, stream, len, info.flags, info.offset);
   }
   FS.munmap(stream);
  }
  SYSCALLS.mappings[addr] = null;
  if (info.allocated) {
   _free(info.malloc);
  }
 }
 return 0;
}

function ___sys_munmap(addr, len) {
 try {
  return syscallMunmap(addr, len);
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_open(path, flags, varargs) {
 SYSCALLS.varargs = varargs;
 try {
  var pathname = SYSCALLS.getStr(path);
  var mode = varargs ? SYSCALLS.get() : 0;
  var stream = FS.open(pathname, flags, mode);
  return stream.fd;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

var ___sys_prlimit64 = function(pid, resource, new_limit, old_limit) {
 err("warning: unsupported syscall: __sys_prlimit64");
 try {
  if (old_limit) {
   _asan_js_store_4(old_limit >> 2, -1);
   _asan_js_store_4(old_limit + 4 >> 2, -1);
   _asan_js_store_4(old_limit + 8 >> 2, -1);
   _asan_js_store_4(old_limit + 12 >> 2, -1);
  }
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
};

var SOCKFS = {
 mount: function(mount) {
  Module["websocket"] = Module["websocket"] && "object" === typeof Module["websocket"] ? Module["websocket"] : {};
  Module["websocket"]._callbacks = {};
  Module["websocket"]["on"] = function(event, callback) {
   if ("function" === typeof callback) {
    this._callbacks[event] = callback;
   }
   return this;
  };
  Module["websocket"].emit = function(event, param) {
   if ("function" === typeof this._callbacks[event]) {
    this._callbacks[event].call(this, param);
   }
  };
  return FS.createNode(null, "/", 16384 | 511, 0);
 },
 createSocket: function(family, type, protocol) {
  type &= ~526336;
  var streaming = type == 1;
  if (protocol) {
   assert(streaming == (protocol == 6));
  }
  var sock = {
   family: family,
   type: type,
   protocol: protocol,
   server: null,
   error: null,
   peers: {},
   pending: [],
   recv_queue: [],
   sock_ops: SOCKFS.websocket_sock_ops
  };
  var name = SOCKFS.nextname();
  var node = FS.createNode(SOCKFS.root, name, 49152, 0);
  node.sock = sock;
  var stream = FS.createStream({
   path: name,
   node: node,
   flags: 2,
   seekable: false,
   stream_ops: SOCKFS.stream_ops
  });
  sock.stream = stream;
  return sock;
 },
 getSocket: function(fd) {
  var stream = FS.getStream(fd);
  if (!stream || !FS.isSocket(stream.node.mode)) {
   return null;
  }
  return stream.node.sock;
 },
 stream_ops: {
  poll: function(stream) {
   var sock = stream.node.sock;
   return sock.sock_ops.poll(sock);
  },
  ioctl: function(stream, request, varargs) {
   var sock = stream.node.sock;
   return sock.sock_ops.ioctl(sock, request, varargs);
  },
  read: function(stream, buffer, offset, length, position) {
   var sock = stream.node.sock;
   var msg = sock.sock_ops.recvmsg(sock, length);
   if (!msg) {
    return 0;
   }
   buffer.set(msg.buffer, offset);
   return msg.buffer.length;
  },
  write: function(stream, buffer, offset, length, position) {
   var sock = stream.node.sock;
   return sock.sock_ops.sendmsg(sock, buffer, offset, length);
  },
  close: function(stream) {
   var sock = stream.node.sock;
   sock.sock_ops.close(sock);
  }
 },
 nextname: function() {
  if (!SOCKFS.nextname.current) {
   SOCKFS.nextname.current = 0;
  }
  return "socket[" + SOCKFS.nextname.current++ + "]";
 },
 websocket_sock_ops: {
  createPeer: function(sock, addr, port) {
   var ws;
   if (typeof addr === "object") {
    ws = addr;
    addr = null;
    port = null;
   }
   if (ws) {
    if (ws._socket) {
     addr = ws._socket.remoteAddress;
     port = ws._socket.remotePort;
    } else {
     var result = /ws[s]?:\/\/([^:]+):(\d+)/.exec(ws.url);
     if (!result) {
      throw new Error("WebSocket URL must be in the format ws(s)://address:port");
     }
     addr = result[1];
     port = parseInt(result[2], 10);
    }
   } else {
    try {
     var runtimeConfig = Module["websocket"] && "object" === typeof Module["websocket"];
     var url = "ws:#".replace("#", "//");
     if (runtimeConfig) {
      if ("string" === typeof Module["websocket"]["url"]) {
       url = Module["websocket"]["url"];
      }
     }
     if (url === "ws://" || url === "wss://") {
      var parts = addr.split("/");
      url = url + parts[0] + ":" + port + "/" + parts.slice(1).join("/");
     }
     var subProtocols = "binary";
     if (runtimeConfig) {
      if ("string" === typeof Module["websocket"]["subprotocol"]) {
       subProtocols = Module["websocket"]["subprotocol"];
      }
     }
     var opts = undefined;
     if (subProtocols !== "null") {
      subProtocols = subProtocols.replace(/^ +| +$/g, "").split(/ *, */);
      opts = ENVIRONMENT_IS_NODE ? {
       "protocol": subProtocols.toString()
      } : subProtocols;
     }
     if (runtimeConfig && null === Module["websocket"]["subprotocol"]) {
      subProtocols = "null";
      opts = undefined;
     }
     var WebSocketConstructor;
     if (ENVIRONMENT_IS_NODE) {
      WebSocketConstructor = require("ws");
     } else {
      WebSocketConstructor = WebSocket;
     }
     ws = new WebSocketConstructor(url, opts);
     ws.binaryType = "arraybuffer";
    } catch (e) {
     throw new FS.ErrnoError(23);
    }
   }
   var peer = {
    addr: addr,
    port: port,
    socket: ws,
    dgram_send_queue: []
   };
   SOCKFS.websocket_sock_ops.addPeer(sock, peer);
   SOCKFS.websocket_sock_ops.handlePeerEvents(sock, peer);
   if (sock.type === 2 && typeof sock.sport !== "undefined") {
    peer.dgram_send_queue.push(new Uint8Array([ 255, 255, 255, 255, "p".charCodeAt(0), "o".charCodeAt(0), "r".charCodeAt(0), "t".charCodeAt(0), (sock.sport & 65280) >> 8, sock.sport & 255 ]));
   }
   return peer;
  },
  getPeer: function(sock, addr, port) {
   return sock.peers[addr + ":" + port];
  },
  addPeer: function(sock, peer) {
   sock.peers[peer.addr + ":" + peer.port] = peer;
  },
  removePeer: function(sock, peer) {
   delete sock.peers[peer.addr + ":" + peer.port];
  },
  handlePeerEvents: function(sock, peer) {
   var first = true;
   var handleOpen = function() {
    Module["websocket"].emit("open", sock.stream.fd);
    try {
     var queued = peer.dgram_send_queue.shift();
     while (queued) {
      peer.socket.send(queued);
      queued = peer.dgram_send_queue.shift();
     }
    } catch (e) {
     peer.socket.close();
    }
   };
   function handleMessage(data) {
    if (typeof data === "string") {
     var encoder = new TextEncoder();
     data = encoder.encode(data);
    } else {
     assert(data.byteLength !== undefined);
     if (data.byteLength == 0) {
      return;
     } else {
      data = new Uint8Array(data);
     }
    }
    var wasfirst = first;
    first = false;
    if (wasfirst && data.length === 10 && data[0] === 255 && data[1] === 255 && data[2] === 255 && data[3] === 255 && data[4] === "p".charCodeAt(0) && data[5] === "o".charCodeAt(0) && data[6] === "r".charCodeAt(0) && data[7] === "t".charCodeAt(0)) {
     var newport = data[8] << 8 | data[9];
     SOCKFS.websocket_sock_ops.removePeer(sock, peer);
     peer.port = newport;
     SOCKFS.websocket_sock_ops.addPeer(sock, peer);
     return;
    }
    sock.recv_queue.push({
     addr: peer.addr,
     port: peer.port,
     data: data
    });
    Module["websocket"].emit("message", sock.stream.fd);
   }
   if (ENVIRONMENT_IS_NODE) {
    peer.socket.on("open", handleOpen);
    peer.socket.on("message", function(data, flags) {
     if (!flags.binary) {
      return;
     }
     handleMessage(new Uint8Array(data).buffer);
    });
    peer.socket.on("close", function() {
     Module["websocket"].emit("close", sock.stream.fd);
    });
    peer.socket.on("error", function(error) {
     sock.error = 14;
     Module["websocket"].emit("error", [ sock.stream.fd, sock.error, "ECONNREFUSED: Connection refused" ]);
    });
   } else {
    peer.socket.onopen = handleOpen;
    peer.socket.onclose = function() {
     Module["websocket"].emit("close", sock.stream.fd);
    };
    peer.socket.onmessage = function peer_socket_onmessage(event) {
     handleMessage(event.data);
    };
    peer.socket.onerror = function(error) {
     sock.error = 14;
     Module["websocket"].emit("error", [ sock.stream.fd, sock.error, "ECONNREFUSED: Connection refused" ]);
    };
   }
  },
  poll: function(sock) {
   if (sock.type === 1 && sock.server) {
    return sock.pending.length ? 64 | 1 : 0;
   }
   var mask = 0;
   var dest = sock.type === 1 ? SOCKFS.websocket_sock_ops.getPeer(sock, sock.daddr, sock.dport) : null;
   if (sock.recv_queue.length || !dest || dest && dest.socket.readyState === dest.socket.CLOSING || dest && dest.socket.readyState === dest.socket.CLOSED) {
    mask |= 64 | 1;
   }
   if (!dest || dest && dest.socket.readyState === dest.socket.OPEN) {
    mask |= 4;
   }
   if (dest && dest.socket.readyState === dest.socket.CLOSING || dest && dest.socket.readyState === dest.socket.CLOSED) {
    mask |= 16;
   }
   return mask;
  },
  ioctl: function(sock, request, arg) {
   switch (request) {
   case 21531:
    var bytes = 0;
    if (sock.recv_queue.length) {
     bytes = sock.recv_queue[0].data.length;
    }
    _asan_js_store_4(arg >> 2, bytes);
    return 0;

   default:
    return 28;
   }
  },
  close: function(sock) {
   if (sock.server) {
    try {
     sock.server.close();
    } catch (e) {}
    sock.server = null;
   }
   var peers = Object.keys(sock.peers);
   for (var i = 0; i < peers.length; i++) {
    var peer = sock.peers[peers[i]];
    try {
     peer.socket.close();
    } catch (e) {}
    SOCKFS.websocket_sock_ops.removePeer(sock, peer);
   }
   return 0;
  },
  bind: function(sock, addr, port) {
   if (typeof sock.saddr !== "undefined" || typeof sock.sport !== "undefined") {
    throw new FS.ErrnoError(28);
   }
   sock.saddr = addr;
   sock.sport = port;
   if (sock.type === 2) {
    if (sock.server) {
     sock.server.close();
     sock.server = null;
    }
    try {
     sock.sock_ops.listen(sock, 0);
    } catch (e) {
     if (!(e instanceof FS.ErrnoError)) throw e;
     if (e.errno !== 138) throw e;
    }
   }
  },
  connect: function(sock, addr, port) {
   if (sock.server) {
    throw new FS.ErrnoError(138);
   }
   if (typeof sock.daddr !== "undefined" && typeof sock.dport !== "undefined") {
    var dest = SOCKFS.websocket_sock_ops.getPeer(sock, sock.daddr, sock.dport);
    if (dest) {
     if (dest.socket.readyState === dest.socket.CONNECTING) {
      throw new FS.ErrnoError(7);
     } else {
      throw new FS.ErrnoError(30);
     }
    }
   }
   var peer = SOCKFS.websocket_sock_ops.createPeer(sock, addr, port);
   sock.daddr = peer.addr;
   sock.dport = peer.port;
   throw new FS.ErrnoError(26);
  },
  listen: function(sock, backlog) {
   if (!ENVIRONMENT_IS_NODE) {
    throw new FS.ErrnoError(138);
   }
   if (sock.server) {
    throw new FS.ErrnoError(28);
   }
   var WebSocketServer = require("ws").Server;
   var host = sock.saddr;
   sock.server = new WebSocketServer({
    host: host,
    port: sock.sport
   });
   Module["websocket"].emit("listen", sock.stream.fd);
   sock.server.on("connection", function(ws) {
    if (sock.type === 1) {
     var newsock = SOCKFS.createSocket(sock.family, sock.type, sock.protocol);
     var peer = SOCKFS.websocket_sock_ops.createPeer(newsock, ws);
     newsock.daddr = peer.addr;
     newsock.dport = peer.port;
     sock.pending.push(newsock);
     Module["websocket"].emit("connection", newsock.stream.fd);
    } else {
     SOCKFS.websocket_sock_ops.createPeer(sock, ws);
     Module["websocket"].emit("connection", sock.stream.fd);
    }
   });
   sock.server.on("closed", function() {
    Module["websocket"].emit("close", sock.stream.fd);
    sock.server = null;
   });
   sock.server.on("error", function(error) {
    sock.error = 23;
    Module["websocket"].emit("error", [ sock.stream.fd, sock.error, "EHOSTUNREACH: Host is unreachable" ]);
   });
  },
  accept: function(listensock) {
   if (!listensock.server) {
    throw new FS.ErrnoError(28);
   }
   var newsock = listensock.pending.shift();
   newsock.stream.flags = listensock.stream.flags;
   return newsock;
  },
  getname: function(sock, peer) {
   var addr, port;
   if (peer) {
    if (sock.daddr === undefined || sock.dport === undefined) {
     throw new FS.ErrnoError(53);
    }
    addr = sock.daddr;
    port = sock.dport;
   } else {
    addr = sock.saddr || 0;
    port = sock.sport || 0;
   }
   return {
    addr: addr,
    port: port
   };
  },
  sendmsg: function(sock, buffer, offset, length, addr, port) {
   if (sock.type === 2) {
    if (addr === undefined || port === undefined) {
     addr = sock.daddr;
     port = sock.dport;
    }
    if (addr === undefined || port === undefined) {
     throw new FS.ErrnoError(17);
    }
   } else {
    addr = sock.daddr;
    port = sock.dport;
   }
   var dest = SOCKFS.websocket_sock_ops.getPeer(sock, addr, port);
   if (sock.type === 1) {
    if (!dest || dest.socket.readyState === dest.socket.CLOSING || dest.socket.readyState === dest.socket.CLOSED) {
     throw new FS.ErrnoError(53);
    } else if (dest.socket.readyState === dest.socket.CONNECTING) {
     throw new FS.ErrnoError(6);
    }
   }
   if (ArrayBuffer.isView(buffer)) {
    offset += buffer.byteOffset;
    buffer = buffer.buffer;
   }
   var data;
   data = buffer.slice(offset, offset + length);
   if (sock.type === 2) {
    if (!dest || dest.socket.readyState !== dest.socket.OPEN) {
     if (!dest || dest.socket.readyState === dest.socket.CLOSING || dest.socket.readyState === dest.socket.CLOSED) {
      dest = SOCKFS.websocket_sock_ops.createPeer(sock, addr, port);
     }
     dest.dgram_send_queue.push(data);
     return length;
    }
   }
   try {
    dest.socket.send(data);
    return length;
   } catch (e) {
    throw new FS.ErrnoError(28);
   }
  },
  recvmsg: function(sock, length) {
   if (sock.type === 1 && sock.server) {
    throw new FS.ErrnoError(53);
   }
   var queued = sock.recv_queue.shift();
   if (!queued) {
    if (sock.type === 1) {
     var dest = SOCKFS.websocket_sock_ops.getPeer(sock, sock.daddr, sock.dport);
     if (!dest) {
      throw new FS.ErrnoError(53);
     } else if (dest.socket.readyState === dest.socket.CLOSING || dest.socket.readyState === dest.socket.CLOSED) {
      return null;
     } else {
      throw new FS.ErrnoError(6);
     }
    } else {
     throw new FS.ErrnoError(6);
    }
   }
   var queuedLength = queued.data.byteLength || queued.data.length;
   var queuedOffset = queued.data.byteOffset || 0;
   var queuedBuffer = queued.data.buffer || queued.data;
   var bytesRead = Math.min(length, queuedLength);
   var res = {
    buffer: new Uint8Array(queuedBuffer, queuedOffset, bytesRead),
    addr: queued.addr,
    port: queued.port
   };
   if (sock.type === 1 && bytesRead < queuedLength) {
    var bytesRemaining = queuedLength - bytesRead;
    queued.data = new Uint8Array(queuedBuffer, queuedOffset + bytesRead, bytesRemaining);
    sock.recv_queue.unshift(queued);
   }
   return res;
  }
 }
};

function getSocketFromFD(fd) {
 var socket = SOCKFS.getSocket(fd);
 if (!socket) throw new FS.ErrnoError(8);
 return socket;
}

var Sockets = {
 BUFFER_SIZE: 10240,
 MAX_BUFFER_SIZE: 10485760,
 nextFd: 1,
 fds: {},
 nextport: 1,
 maxport: 65535,
 peer: null,
 connections: {},
 portmap: {},
 localAddr: 4261412874,
 addrPool: [ 33554442, 50331658, 67108874, 83886090, 100663306, 117440522, 134217738, 150994954, 167772170, 184549386, 201326602, 218103818, 234881034 ]
};

function inetPton4(str) {
 var b = str.split(".");
 for (var i = 0; i < 4; i++) {
  var tmp = Number(b[i]);
  if (isNaN(tmp)) return null;
  b[i] = tmp;
 }
 return (b[0] | b[1] << 8 | b[2] << 16 | b[3] << 24) >>> 0;
}

function jstoi_q(str) {
 return parseInt(str);
}

function inetPton6(str) {
 var words;
 var w, offset, z, i;
 var valid6regx = /^((?=.*::)(?!.*::.+::)(::)?([\dA-F]{1,4}:(:|\b)|){5}|([\dA-F]{1,4}:){6})((([\dA-F]{1,4}((?!\3)::|:\b|$))|(?!\2\3)){2}|(((2[0-4]|1\d|[1-9])?\d|25[0-5])\.?\b){4})$/i;
 var parts = [];
 if (!valid6regx.test(str)) {
  return null;
 }
 if (str === "::") {
  return [ 0, 0, 0, 0, 0, 0, 0, 0 ];
 }
 if (str.startsWith("::")) {
  str = str.replace("::", "Z:");
 } else {
  str = str.replace("::", ":Z:");
 }
 if (str.indexOf(".") > 0) {
  str = str.replace(new RegExp("[.]", "g"), ":");
  words = str.split(":");
  words[words.length - 4] = jstoi_q(words[words.length - 4]) + jstoi_q(words[words.length - 3]) * 256;
  words[words.length - 3] = jstoi_q(words[words.length - 2]) + jstoi_q(words[words.length - 1]) * 256;
  words = words.slice(0, words.length - 2);
 } else {
  words = str.split(":");
 }
 offset = 0;
 z = 0;
 for (w = 0; w < words.length; w++) {
  if (typeof words[w] === "string") {
   if (words[w] === "Z") {
    for (z = 0; z < 8 - words.length + 1; z++) {
     parts[w + z] = 0;
    }
    offset = z - 1;
   } else {
    parts[w + offset] = _htons(parseInt(words[w], 16));
   }
  } else {
   parts[w + offset] = words[w];
  }
 }
 return [ parts[1] << 16 | parts[0], parts[3] << 16 | parts[2], parts[5] << 16 | parts[4], parts[7] << 16 | parts[6] ];
}

function writeSockaddr(sa, family, addr, port, addrlen) {
 switch (family) {
 case 2:
  addr = inetPton4(addr);
  zeroMemory(sa, 16);
  if (addrlen) {
   _asan_js_store_4(addrlen >> 2, 16);
  }
  _asan_js_store_2(sa >> 1, family);
  _asan_js_store_4(sa + 4 >> 2, addr);
  _asan_js_store_2(sa + 2 >> 1, _htons(port));
  break;

 case 10:
  addr = inetPton6(addr);
  zeroMemory(sa, 28);
  if (addrlen) {
   _asan_js_store_4(addrlen >> 2, 28);
  }
  _asan_js_store_4(sa >> 2, family);
  _asan_js_store_4(sa + 8 >> 2, addr[0]);
  _asan_js_store_4(sa + 12 >> 2, addr[1]);
  _asan_js_store_4(sa + 16 >> 2, addr[2]);
  _asan_js_store_4(sa + 20 >> 2, addr[3]);
  _asan_js_store_2(sa + 2 >> 1, _htons(port));
  break;

 default:
  return 5;
 }
 return 0;
}

var DNS = {
 address_map: {
  id: 1,
  addrs: {},
  names: {}
 },
 lookup_name: function(name) {
  var res = inetPton4(name);
  if (res !== null) {
   return name;
  }
  res = inetPton6(name);
  if (res !== null) {
   return name;
  }
  var addr;
  if (DNS.address_map.addrs[name]) {
   addr = DNS.address_map.addrs[name];
  } else {
   var id = DNS.address_map.id++;
   assert(id < 65535, "exceeded max address mappings of 65535");
   addr = "172.29." + (id & 255) + "." + (id & 65280);
   DNS.address_map.names[addr] = name;
   DNS.address_map.addrs[name] = addr;
  }
  return addr;
 },
 lookup_addr: function(addr) {
  if (DNS.address_map.names[addr]) {
   return DNS.address_map.names[addr];
  }
  return null;
 }
};

function ___sys_recvfrom(fd, buf, len, flags, addr, addrlen) {
 try {
  var sock = getSocketFromFD(fd);
  var msg = sock.sock_ops.recvmsg(sock, len);
  if (!msg) return 0;
  if (addr) {
   var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(msg.addr), msg.port, addrlen);
   assert(!errno);
  }
  HEAPU8.set(msg.buffer, buf);
  return msg.buffer.byteLength;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_rename(old_path, new_path) {
 try {
  old_path = SYSCALLS.getStr(old_path);
  new_path = SYSCALLS.getStr(new_path);
  FS.rename(old_path, new_path);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_rmdir(path) {
 try {
  path = SYSCALLS.getStr(path);
  FS.rmdir(path);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function inetNtop4(addr) {
 return (addr & 255) + "." + (addr >> 8 & 255) + "." + (addr >> 16 & 255) + "." + (addr >> 24 & 255);
}

function inetNtop6(ints) {
 var str = "";
 var word = 0;
 var longest = 0;
 var lastzero = 0;
 var zstart = 0;
 var len = 0;
 var i = 0;
 var parts = [ ints[0] & 65535, ints[0] >> 16, ints[1] & 65535, ints[1] >> 16, ints[2] & 65535, ints[2] >> 16, ints[3] & 65535, ints[3] >> 16 ];
 var hasipv4 = true;
 var v4part = "";
 for (i = 0; i < 5; i++) {
  if (parts[i] !== 0) {
   hasipv4 = false;
   break;
  }
 }
 if (hasipv4) {
  v4part = inetNtop4(parts[6] | parts[7] << 16);
  if (parts[5] === -1) {
   str = "::ffff:";
   str += v4part;
   return str;
  }
  if (parts[5] === 0) {
   str = "::";
   if (v4part === "0.0.0.0") v4part = "";
   if (v4part === "0.0.0.1") v4part = "1";
   str += v4part;
   return str;
  }
 }
 for (word = 0; word < 8; word++) {
  if (parts[word] === 0) {
   if (word - lastzero > 1) {
    len = 0;
   }
   lastzero = word;
   len++;
  }
  if (len > longest) {
   longest = len;
   zstart = word - longest + 1;
  }
 }
 for (word = 0; word < 8; word++) {
  if (longest > 1) {
   if (parts[word] === 0 && word >= zstart && word < zstart + longest) {
    if (word === zstart) {
     str += ":";
     if (zstart === 0) str += ":";
    }
    continue;
   }
  }
  str += Number(_ntohs(parts[word] & 65535)).toString(16);
  str += word < 7 ? ":" : "";
 }
 return str;
}

function readSockaddr(sa, salen) {
 var family = _asan_js_load_2(sa >> 1);
 var port = _ntohs(_asan_js_load_2u(sa + 2 >> 1));
 var addr;
 switch (family) {
 case 2:
  if (salen !== 16) {
   return {
    errno: 28
   };
  }
  addr = _asan_js_load_4(sa + 4 >> 2);
  addr = inetNtop4(addr);
  break;

 case 10:
  if (salen !== 28) {
   return {
    errno: 28
   };
  }
  addr = [ _asan_js_load_4(sa + 8 >> 2), _asan_js_load_4(sa + 12 >> 2), _asan_js_load_4(sa + 16 >> 2), _asan_js_load_4(sa + 20 >> 2) ];
  addr = inetNtop6(addr);
  break;

 default:
  return {
   errno: 5
  };
 }
 return {
  family: family,
  addr: addr,
  port: port
 };
}

function getSocketAddress(addrp, addrlen, allowNull) {
 if (allowNull && addrp === 0) return null;
 var info = readSockaddr(addrp, addrlen);
 if (info.errno) throw new FS.ErrnoError(info.errno);
 info.addr = DNS.lookup_addr(info.addr) || info.addr;
 return info;
}

function ___sys_sendto(fd, message, length, flags, addr, addr_len) {
 try {
  var sock = getSocketFromFD(fd);
  var dest = getSocketAddress(addr, addr_len, true);
  if (!dest) {
   return FS.write(sock.stream, HEAP8, message, length);
  } else {
   return sock.sock_ops.sendmsg(sock, HEAP8, message, length, dest.addr, dest.port);
  }
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_setrlimit(varargs) {
 return 0;
}

var ___sys_shutdown = function(fd, how) {
 err("warning: unsupported syscall: __sys_shutdown");
 try {
  getSocketFromFD(fd);
  return -52;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
};

function ___sys_stat64(path, buf) {
 try {
  path = SYSCALLS.getStr(path);
  return SYSCALLS.doStat(FS.stat, path, buf);
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

var ___sys_ugetrlimit = function(resource, rlim) {
 err("warning: unsupported syscall: __sys_ugetrlimit");
 try {
  _asan_js_store_4(rlim >> 2, -1);
  _asan_js_store_4(rlim + 4 >> 2, -1);
  _asan_js_store_4(rlim + 8 >> 2, -1);
  _asan_js_store_4(rlim + 12 >> 2, -1);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
};

function ___sys_uname(buf) {
 try {
  if (!buf) return -21;
  var layout = {
   "__size__": 390,
   "domainname": 325,
   "machine": 260,
   "nodename": 65,
   "release": 130,
   "sysname": 0,
   "version": 195
  };
  var copyString = function(element, value) {
   var offset = layout[element];
   writeAsciiToMemory(value, buf + offset);
  };
  copyString("sysname", "Emscripten");
  copyString("nodename", "emscripten");
  copyString("release", "1.0");
  copyString("version", "#1");
  copyString("machine", "wasm32");
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function ___sys_unlink(path) {
 try {
  path = SYSCALLS.getStr(path);
  FS.unlink(path);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return -e.errno;
 }
}

function __dlopen_js(filename, flag) {
 abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
}

function __dlsym_js(handle, symbol) {
 abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
}

function _abort() {
 abort("native code called abort()");
}

var readAsmConstArgsArray = [];

function readAsmConstArgs(sigPtr, buf) {
 assert(Array.isArray(readAsmConstArgsArray));
 assert(buf % 16 == 0);
 readAsmConstArgsArray.length = 0;
 var ch;
 buf >>= 2;
 while (ch = _asan_js_load_1u(sigPtr++)) {
  assert(ch === 100 || ch === 102 || ch === 105);
  var readAsmConstArgsDouble = ch < 105;
  if (readAsmConstArgsDouble && buf & 1) buf++;
  readAsmConstArgsArray.push(readAsmConstArgsDouble ? _asan_js_load_d(buf++ >> 1) : _asan_js_load_4(buf));
  ++buf;
 }
 return readAsmConstArgsArray;
}

function _emscripten_asm_const_int(code, sigPtr, argbuf) {
 var args = readAsmConstArgs(sigPtr, argbuf);
 if (!ASM_CONSTS.hasOwnProperty(code)) abort("No EM_ASM constant found at address " + code);
 return ASM_CONSTS[code].apply(null, args);
}

function withBuiltinMalloc(func) {
 var prev_malloc = typeof _malloc !== "undefined" ? _malloc : undefined;
 var prev_memalign = typeof _memalign !== "undefined" ? _memalign : undefined;
 var prev_free = typeof _free !== "undefined" ? _free : undefined;
 _malloc = _emscripten_builtin_malloc;
 _memalign = _emscripten_builtin_memalign;
 _free = _emscripten_builtin_free;
 var prev_memset = typeof _memset !== "undefined" ? _memset : undefined;
 _memset = _emscripten_builtin_memset;
 try {
  return func();
 } finally {
  _malloc = prev_malloc;
  _memalign = prev_memalign;
  _free = prev_free;
  _memset = prev_memset;
 }
}

function _emscripten_builtin_mmap2(addr, len, prot, flags, fd, off) {
 return withBuiltinMalloc(function() {
  return syscallMmap2(addr, len, prot, flags, fd, off);
 });
}

function _emscripten_builtin_munmap(addr, len) {
 return withBuiltinMalloc(function() {
  return syscallMunmap(addr, len);
 });
}

function _emscripten_cancel_main_loop() {
 Browser.mainLoop.pause();
 Browser.mainLoop.func = null;
}

function _emscripten_get_heap_max() {
 return 2147483648;
}

function _emscripten_get_module_name(buf, length) {
 return stringToUTF8(wasmBinaryFile, buf, length);
}

var UNWIND_CACHE = {};

function _emscripten_generate_pc(frame) {
 assert(wasmOffsetConverter);
 var match;
 if (match = /\bwasm-function\[\d+\]:(0x[0-9a-f]+)/.exec(frame)) {
  return +match[1];
 } else if (match = /\bwasm-function\[(\d+)\]:(\d+)/.exec(frame)) {
  return wasmOffsetConverter.convert(+match[1], +match[2]);
 } else if (match = /:(\d+):\d+(?:\)|$)/.exec(frame)) {
  return 2147483648 | +match[1];
 } else {
  return 0;
 }
}

function _emscripten_pc_get_source_js(pc) {
 if (UNWIND_CACHE.last_get_source_pc == pc) return UNWIND_CACHE.last_source;
 var match;
 var source;
 if (!source) {
  var frame = UNWIND_CACHE[pc];
  if (!frame) return null;
  if (match = /\((.*):(\d+):(\d+)\)$/.exec(frame)) {
   source = {
    file: match[1],
    line: match[2],
    column: match[3]
   };
  } else if (match = /@(.*):(\d+):(\d+)/.exec(frame)) {
   source = {
    file: match[1],
    line: match[2],
    column: match[3]
   };
  }
 }
 UNWIND_CACHE.last_get_source_pc = pc;
 UNWIND_CACHE.last_source = source;
 return source;
}

function _emscripten_pc_get_column(pc) {
 var result = _emscripten_pc_get_source_js(pc);
 return result ? result.column || 0 : 0;
}

function _emscripten_pc_get_file(pc) {
 return withBuiltinMalloc(function() {
  var result = _emscripten_pc_get_source_js(pc);
  if (!result) return 0;
  if (_emscripten_pc_get_file.ret) _free(_emscripten_pc_get_file.ret);
  _emscripten_pc_get_file.ret = allocateUTF8(result.file);
  return _emscripten_pc_get_file.ret;
 });
}

function _emscripten_pc_get_function(pc) {
 return withBuiltinMalloc(function() {
  var name;
  if (pc & 2147483648) {
   var frame = UNWIND_CACHE[pc];
   if (!frame) return 0;
   var match;
   if (match = /^\s+at (.*) \(.*\)$/.exec(frame)) {
    name = match[1];
   } else if (match = /^(.+?)@/.exec(frame)) {
    name = match[1];
   } else {
    return 0;
   }
  } else {
   name = wasmOffsetConverter.getName(pc);
  }
  if (_emscripten_pc_get_function.ret) _free(_emscripten_pc_get_function.ret);
  _emscripten_pc_get_function.ret = allocateUTF8(name);
  return _emscripten_pc_get_function.ret;
 });
}

function _emscripten_pc_get_line(pc) {
 var result = _emscripten_pc_get_source_js(pc);
 return result ? result.line : 0;
}

function emscripten_realloc_buffer(size) {
 try {
  wasmMemory.grow(size - buffer.byteLength + 65535 >>> 16);
  updateGlobalBufferAndViews(wasmMemory.buffer);
  return 1;
 } catch (e) {
  err("emscripten_realloc_buffer: Attempted to grow heap from " + buffer.byteLength + " bytes to " + size + " bytes, but got error: " + e);
 }
}

function _emscripten_resize_heap(requestedSize) {
 var oldSize = HEAPU8.length;
 requestedSize = requestedSize >>> 0;
 assert(requestedSize > oldSize);
 var maxHeapSize = 2147483648;
 if (requestedSize > maxHeapSize) {
  err("Cannot enlarge memory, asked to go up to " + requestedSize + " bytes, but the limit is " + maxHeapSize + " bytes!");
  return false;
 }
 for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
  var overGrownHeapSize = oldSize * (1 + .2 / cutDown);
  overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296);
  var newSize = Math.min(maxHeapSize, alignUp(Math.max(requestedSize, overGrownHeapSize), 65536));
  var replacement = emscripten_realloc_buffer(newSize);
  if (replacement) {
   return true;
  }
 }
 err("Failed to grow the heap from " + oldSize + " bytes to " + newSize + " bytes, not enough memory!");
 return false;
}

function _emscripten_return_address(level) {
 var callstack = new Error().stack.split("\n");
 if (callstack[0] == "Error") {
  callstack.shift();
 }
 return _emscripten_generate_pc(callstack[level + 2]);
}

function _emscripten_set_main_loop(func, fps, simulateInfiniteLoop) {
 var browserIterationFunc = wasmTable.get(func);
 setMainLoop(browserIterationFunc, fps, simulateInfiniteLoop);
}

function __emscripten_save_in_unwind_cache(callstack) {
 callstack.forEach(function(frame) {
  var pc = _emscripten_generate_pc(frame);
  if (pc) {
   UNWIND_CACHE[pc] = frame;
  }
 });
}

function _emscripten_stack_snapshot() {
 var callstack = new Error().stack.split("\n");
 if (callstack[0] == "Error") {
  callstack.shift();
 }
 __emscripten_save_in_unwind_cache(callstack);
 UNWIND_CACHE.last_addr = _emscripten_generate_pc(callstack[2]);
 UNWIND_CACHE.last_stack = callstack;
 return UNWIND_CACHE.last_addr;
}

function _emscripten_stack_unwind_buffer(addr, buffer, count) {
 var stack;
 if (UNWIND_CACHE.last_addr == addr) {
  stack = UNWIND_CACHE.last_stack;
 } else {
  stack = new Error().stack.split("\n");
  if (stack[0] == "Error") {
   stack.shift();
  }
  __emscripten_save_in_unwind_cache(stack);
 }
 var offset = 2;
 while (stack[offset] && _emscripten_generate_pc(stack[offset]) != addr) {
  ++offset;
 }
 for (var i = 0; i < count && stack[i + offset]; ++i) {
  _asan_js_store_4(buffer + i * 4 >> 2, _emscripten_generate_pc(stack[i + offset]));
 }
 return i;
}

function _emscripten_thread_sleep(msecs) {
 var start = _emscripten_get_now();
 while (_emscripten_get_now() - start < msecs) {}
}

var ENV = {};

function getExecutableName() {
 return thisProgram || "./this.program";
}

function getEnvStrings() {
 if (!getEnvStrings.strings) {
  var lang = (typeof navigator === "object" && navigator.languages && navigator.languages[0] || "C").replace("-", "_") + ".UTF-8";
  var env = {
   "USER": "web_user",
   "LOGNAME": "web_user",
   "PATH": "/",
   "PWD": "/",
   "HOME": "/home/web_user",
   "LANG": lang,
   "_": getExecutableName()
  };
  for (var x in ENV) {
   if (ENV[x] === undefined) delete env[x]; else env[x] = ENV[x];
  }
  var strings = [];
  for (var x in env) {
   strings.push(x + "=" + env[x]);
  }
  getEnvStrings.strings = strings;
 }
 return getEnvStrings.strings;
}

function _environ_get(__environ, environ_buf) {
 var bufSize = 0;
 getEnvStrings().forEach(function(string, i) {
  var ptr = environ_buf + bufSize;
  _asan_js_store_4(__environ + i * 4 >> 2, ptr);
  writeAsciiToMemory(string, ptr);
  bufSize += string.length + 1;
 });
 return 0;
}

function _environ_sizes_get(penviron_count, penviron_buf_size) {
 var strings = getEnvStrings();
 _asan_js_store_4(penviron_count >> 2, strings.length);
 var bufSize = 0;
 strings.forEach(function(string) {
  bufSize += string.length + 1;
 });
 _asan_js_store_4(penviron_buf_size >> 2, bufSize);
 return 0;
}

function _fd_close(fd) {
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  FS.close(stream);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return e.errno;
 }
}

function _fd_fdstat_get(fd, pbuf) {
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  var type = stream.tty ? 2 : FS.isDir(stream.mode) ? 3 : FS.isLink(stream.mode) ? 7 : 4;
  _asan_js_store_1(pbuf >> 0, type);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return e.errno;
 }
}

function _fd_read(fd, iov, iovcnt, pnum) {
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  var num = SYSCALLS.doReadv(stream, iov, iovcnt);
  _asan_js_store_4(pnum >> 2, num);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return e.errno;
 }
}

function _fd_seek(fd, offset_low, offset_high, whence, newOffset) {
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  var HIGH_OFFSET = 4294967296;
  var offset = offset_high * HIGH_OFFSET + (offset_low >>> 0);
  var DOUBLE_LIMIT = 9007199254740992;
  if (offset <= -DOUBLE_LIMIT || offset >= DOUBLE_LIMIT) {
   return -61;
  }
  FS.llseek(stream, offset, whence);
  tempI64 = [ stream.position >>> 0, (tempDouble = stream.position, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0) ], 
  _asan_js_store_4(newOffset >> 2, tempI64[0]), _asan_js_store_4(newOffset + 4 >> 2, tempI64[1]);
  if (stream.getdents && offset === 0 && whence === 0) stream.getdents = null;
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return e.errno;
 }
}

function _fd_write(fd, iov, iovcnt, pnum) {
 try {
  var stream = SYSCALLS.getStreamFromFD(fd);
  var num = SYSCALLS.doWritev(stream, iov, iovcnt);
  _asan_js_store_4(pnum >> 2, num);
  return 0;
 } catch (e) {
  if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
  return e.errno;
 }
}

function _getTempRet0() {
 return getTempRet0();
}

function _gettimeofday(ptr) {
 var now = Date.now();
 _asan_js_store_4(ptr >> 2, now / 1e3 | 0);
 _asan_js_store_4(ptr + 4 >> 2, now % 1e3 * 1e3 | 0);
 return 0;
}

function _proc_exit(code) {
 procExit(code);
}

function _setTempRet0(val) {
 setTempRet0(val);
}

function __isLeapYear(year) {
 return year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0);
}

function __arraySum(array, index) {
 var sum = 0;
 for (var i = 0; i <= index; sum += array[i++]) {}
 return sum;
}

var __MONTH_DAYS_LEAP = [ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 ];

var __MONTH_DAYS_REGULAR = [ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 ];

function __addDays(date, days) {
 var newDate = new Date(date.getTime());
 while (days > 0) {
  var leap = __isLeapYear(newDate.getFullYear());
  var currentMonth = newDate.getMonth();
  var daysInCurrentMonth = (leap ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR)[currentMonth];
  if (days > daysInCurrentMonth - newDate.getDate()) {
   days -= daysInCurrentMonth - newDate.getDate() + 1;
   newDate.setDate(1);
   if (currentMonth < 11) {
    newDate.setMonth(currentMonth + 1);
   } else {
    newDate.setMonth(0);
    newDate.setFullYear(newDate.getFullYear() + 1);
   }
  } else {
   newDate.setDate(newDate.getDate() + days);
   return newDate;
  }
 }
 return newDate;
}

function _strftime(s, maxsize, format, tm) {
 var tm_zone = _asan_js_load_4(tm + 40 >> 2);
 var date = {
  tm_sec: _asan_js_load_4(tm >> 2),
  tm_min: _asan_js_load_4(tm + 4 >> 2),
  tm_hour: _asan_js_load_4(tm + 8 >> 2),
  tm_mday: _asan_js_load_4(tm + 12 >> 2),
  tm_mon: _asan_js_load_4(tm + 16 >> 2),
  tm_year: _asan_js_load_4(tm + 20 >> 2),
  tm_wday: _asan_js_load_4(tm + 24 >> 2),
  tm_yday: _asan_js_load_4(tm + 28 >> 2),
  tm_isdst: _asan_js_load_4(tm + 32 >> 2),
  tm_gmtoff: _asan_js_load_4(tm + 36 >> 2),
  tm_zone: tm_zone ? UTF8ToString(tm_zone) : ""
 };
 var pattern = UTF8ToString(format);
 var EXPANSION_RULES_1 = {
  "%c": "%a %b %d %H:%M:%S %Y",
  "%D": "%m/%d/%y",
  "%F": "%Y-%m-%d",
  "%h": "%b",
  "%r": "%I:%M:%S %p",
  "%R": "%H:%M",
  "%T": "%H:%M:%S",
  "%x": "%m/%d/%y",
  "%X": "%H:%M:%S",
  "%Ec": "%c",
  "%EC": "%C",
  "%Ex": "%m/%d/%y",
  "%EX": "%H:%M:%S",
  "%Ey": "%y",
  "%EY": "%Y",
  "%Od": "%d",
  "%Oe": "%e",
  "%OH": "%H",
  "%OI": "%I",
  "%Om": "%m",
  "%OM": "%M",
  "%OS": "%S",
  "%Ou": "%u",
  "%OU": "%U",
  "%OV": "%V",
  "%Ow": "%w",
  "%OW": "%W",
  "%Oy": "%y"
 };
 for (var rule in EXPANSION_RULES_1) {
  pattern = pattern.replace(new RegExp(rule, "g"), EXPANSION_RULES_1[rule]);
 }
 var WEEKDAYS = [ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" ];
 var MONTHS = [ "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" ];
 function leadingSomething(value, digits, character) {
  var str = typeof value === "number" ? value.toString() : value || "";
  while (str.length < digits) {
   str = character[0] + str;
  }
  return str;
 }
 function leadingNulls(value, digits) {
  return leadingSomething(value, digits, "0");
 }
 function compareByDay(date1, date2) {
  function sgn(value) {
   return value < 0 ? -1 : value > 0 ? 1 : 0;
  }
  var compare;
  if ((compare = sgn(date1.getFullYear() - date2.getFullYear())) === 0) {
   if ((compare = sgn(date1.getMonth() - date2.getMonth())) === 0) {
    compare = sgn(date1.getDate() - date2.getDate());
   }
  }
  return compare;
 }
 function getFirstWeekStartDate(janFourth) {
  switch (janFourth.getDay()) {
  case 0:
   return new Date(janFourth.getFullYear() - 1, 11, 29);

  case 1:
   return janFourth;

  case 2:
   return new Date(janFourth.getFullYear(), 0, 3);

  case 3:
   return new Date(janFourth.getFullYear(), 0, 2);

  case 4:
   return new Date(janFourth.getFullYear(), 0, 1);

  case 5:
   return new Date(janFourth.getFullYear() - 1, 11, 31);

  case 6:
   return new Date(janFourth.getFullYear() - 1, 11, 30);
  }
 }
 function getWeekBasedYear(date) {
  var thisDate = __addDays(new Date(date.tm_year + 1900, 0, 1), date.tm_yday);
  var janFourthThisYear = new Date(thisDate.getFullYear(), 0, 4);
  var janFourthNextYear = new Date(thisDate.getFullYear() + 1, 0, 4);
  var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
  var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);
  if (compareByDay(firstWeekStartThisYear, thisDate) <= 0) {
   if (compareByDay(firstWeekStartNextYear, thisDate) <= 0) {
    return thisDate.getFullYear() + 1;
   } else {
    return thisDate.getFullYear();
   }
  } else {
   return thisDate.getFullYear() - 1;
  }
 }
 var EXPANSION_RULES_2 = {
  "%a": function(date) {
   return WEEKDAYS[date.tm_wday].substring(0, 3);
  },
  "%A": function(date) {
   return WEEKDAYS[date.tm_wday];
  },
  "%b": function(date) {
   return MONTHS[date.tm_mon].substring(0, 3);
  },
  "%B": function(date) {
   return MONTHS[date.tm_mon];
  },
  "%C": function(date) {
   var year = date.tm_year + 1900;
   return leadingNulls(year / 100 | 0, 2);
  },
  "%d": function(date) {
   return leadingNulls(date.tm_mday, 2);
  },
  "%e": function(date) {
   return leadingSomething(date.tm_mday, 2, " ");
  },
  "%g": function(date) {
   return getWeekBasedYear(date).toString().substring(2);
  },
  "%G": function(date) {
   return getWeekBasedYear(date);
  },
  "%H": function(date) {
   return leadingNulls(date.tm_hour, 2);
  },
  "%I": function(date) {
   var twelveHour = date.tm_hour;
   if (twelveHour == 0) twelveHour = 12; else if (twelveHour > 12) twelveHour -= 12;
   return leadingNulls(twelveHour, 2);
  },
  "%j": function(date) {
   return leadingNulls(date.tm_mday + __arraySum(__isLeapYear(date.tm_year + 1900) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, date.tm_mon - 1), 3);
  },
  "%m": function(date) {
   return leadingNulls(date.tm_mon + 1, 2);
  },
  "%M": function(date) {
   return leadingNulls(date.tm_min, 2);
  },
  "%n": function() {
   return "\n";
  },
  "%p": function(date) {
   if (date.tm_hour >= 0 && date.tm_hour < 12) {
    return "AM";
   } else {
    return "PM";
   }
  },
  "%S": function(date) {
   return leadingNulls(date.tm_sec, 2);
  },
  "%t": function() {
   return "\t";
  },
  "%u": function(date) {
   return date.tm_wday || 7;
  },
  "%U": function(date) {
   var janFirst = new Date(date.tm_year + 1900, 0, 1);
   var firstSunday = janFirst.getDay() === 0 ? janFirst : __addDays(janFirst, 7 - janFirst.getDay());
   var endDate = new Date(date.tm_year + 1900, date.tm_mon, date.tm_mday);
   if (compareByDay(firstSunday, endDate) < 0) {
    var februaryFirstUntilEndMonth = __arraySum(__isLeapYear(endDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, endDate.getMonth() - 1) - 31;
    var firstSundayUntilEndJanuary = 31 - firstSunday.getDate();
    var days = firstSundayUntilEndJanuary + februaryFirstUntilEndMonth + endDate.getDate();
    return leadingNulls(Math.ceil(days / 7), 2);
   }
   return compareByDay(firstSunday, janFirst) === 0 ? "01" : "00";
  },
  "%V": function(date) {
   var janFourthThisYear = new Date(date.tm_year + 1900, 0, 4);
   var janFourthNextYear = new Date(date.tm_year + 1901, 0, 4);
   var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
   var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);
   var endDate = __addDays(new Date(date.tm_year + 1900, 0, 1), date.tm_yday);
   if (compareByDay(endDate, firstWeekStartThisYear) < 0) {
    return "53";
   }
   if (compareByDay(firstWeekStartNextYear, endDate) <= 0) {
    return "01";
   }
   var daysDifference;
   if (firstWeekStartThisYear.getFullYear() < date.tm_year + 1900) {
    daysDifference = date.tm_yday + 32 - firstWeekStartThisYear.getDate();
   } else {
    daysDifference = date.tm_yday + 1 - firstWeekStartThisYear.getDate();
   }
   return leadingNulls(Math.ceil(daysDifference / 7), 2);
  },
  "%w": function(date) {
   return date.tm_wday;
  },
  "%W": function(date) {
   var janFirst = new Date(date.tm_year, 0, 1);
   var firstMonday = janFirst.getDay() === 1 ? janFirst : __addDays(janFirst, janFirst.getDay() === 0 ? 1 : 7 - janFirst.getDay() + 1);
   var endDate = new Date(date.tm_year + 1900, date.tm_mon, date.tm_mday);
   if (compareByDay(firstMonday, endDate) < 0) {
    var februaryFirstUntilEndMonth = __arraySum(__isLeapYear(endDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, endDate.getMonth() - 1) - 31;
    var firstMondayUntilEndJanuary = 31 - firstMonday.getDate();
    var days = firstMondayUntilEndJanuary + februaryFirstUntilEndMonth + endDate.getDate();
    return leadingNulls(Math.ceil(days / 7), 2);
   }
   return compareByDay(firstMonday, janFirst) === 0 ? "01" : "00";
  },
  "%y": function(date) {
   return (date.tm_year + 1900).toString().substring(2);
  },
  "%Y": function(date) {
   return date.tm_year + 1900;
  },
  "%z": function(date) {
   var off = date.tm_gmtoff;
   var ahead = off >= 0;
   off = Math.abs(off) / 60;
   off = off / 60 * 100 + off % 60;
   return (ahead ? "+" : "-") + String("0000" + off).slice(-4);
  },
  "%Z": function(date) {
   return date.tm_zone;
  },
  "%%": function() {
   return "%";
  }
 };
 for (var rule in EXPANSION_RULES_2) {
  if (pattern.includes(rule)) {
   pattern = pattern.replace(new RegExp(rule, "g"), EXPANSION_RULES_2[rule](date));
  }
 }
 var bytes = intArrayFromString(pattern, false);
 if (bytes.length > maxsize) {
  return 0;
 }
 writeArrayToMemory(bytes, s);
 return bytes.length - 1;
}

function _strftime_l(s, maxsize, format, tm) {
 return _strftime(s, maxsize, format, tm);
}

function _time(ptr) {
 var ret = Date.now() / 1e3 | 0;
 if (ptr) {
  _asan_js_store_4(ptr >> 2, ret);
 }
 return ret;
}

var FSNode = function(parent, name, mode, rdev) {
 if (!parent) {
  parent = this;
 }
 this.parent = parent;
 this.mount = parent.mount;
 this.mounted = null;
 this.id = FS.nextInode++;
 this.name = name;
 this.mode = mode;
 this.node_ops = {};
 this.stream_ops = {};
 this.rdev = rdev;
};

var readMode = 292 | 73;

var writeMode = 146;

Object.defineProperties(FSNode.prototype, {
 read: {
  get: function() {
   return (this.mode & readMode) === readMode;
  },
  set: function(val) {
   val ? this.mode |= readMode : this.mode &= ~readMode;
  }
 },
 write: {
  get: function() {
   return (this.mode & writeMode) === writeMode;
  },
  set: function(val) {
   val ? this.mode |= writeMode : this.mode &= ~writeMode;
  }
 },
 isFolder: {
  get: function() {
   return FS.isDir(this.mode);
  }
 },
 isDevice: {
  get: function() {
   return FS.isChrdev(this.mode);
  }
 }
});

FS.FSNode = FSNode;

FS.staticInit();

Module["FS_createPath"] = FS.createPath;

Module["FS_createDataFile"] = FS.createDataFile;

Module["FS_createPreloadedFile"] = FS.createPreloadedFile;

Module["FS_createLazyFile"] = FS.createLazyFile;

Module["FS_createDevice"] = FS.createDevice;

Module["FS_unlink"] = FS.unlink;

ERRNO_CODES = {
 "EPERM": 63,
 "ENOENT": 44,
 "ESRCH": 71,
 "EINTR": 27,
 "EIO": 29,
 "ENXIO": 60,
 "E2BIG": 1,
 "ENOEXEC": 45,
 "EBADF": 8,
 "ECHILD": 12,
 "EAGAIN": 6,
 "EWOULDBLOCK": 6,
 "ENOMEM": 48,
 "EACCES": 2,
 "EFAULT": 21,
 "ENOTBLK": 105,
 "EBUSY": 10,
 "EEXIST": 20,
 "EXDEV": 75,
 "ENODEV": 43,
 "ENOTDIR": 54,
 "EISDIR": 31,
 "EINVAL": 28,
 "ENFILE": 41,
 "EMFILE": 33,
 "ENOTTY": 59,
 "ETXTBSY": 74,
 "EFBIG": 22,
 "ENOSPC": 51,
 "ESPIPE": 70,
 "EROFS": 69,
 "EMLINK": 34,
 "EPIPE": 64,
 "EDOM": 18,
 "ERANGE": 68,
 "ENOMSG": 49,
 "EIDRM": 24,
 "ECHRNG": 106,
 "EL2NSYNC": 156,
 "EL3HLT": 107,
 "EL3RST": 108,
 "ELNRNG": 109,
 "EUNATCH": 110,
 "ENOCSI": 111,
 "EL2HLT": 112,
 "EDEADLK": 16,
 "ENOLCK": 46,
 "EBADE": 113,
 "EBADR": 114,
 "EXFULL": 115,
 "ENOANO": 104,
 "EBADRQC": 103,
 "EBADSLT": 102,
 "EDEADLOCK": 16,
 "EBFONT": 101,
 "ENOSTR": 100,
 "ENODATA": 116,
 "ETIME": 117,
 "ENOSR": 118,
 "ENONET": 119,
 "ENOPKG": 120,
 "EREMOTE": 121,
 "ENOLINK": 47,
 "EADV": 122,
 "ESRMNT": 123,
 "ECOMM": 124,
 "EPROTO": 65,
 "EMULTIHOP": 36,
 "EDOTDOT": 125,
 "EBADMSG": 9,
 "ENOTUNIQ": 126,
 "EBADFD": 127,
 "EREMCHG": 128,
 "ELIBACC": 129,
 "ELIBBAD": 130,
 "ELIBSCN": 131,
 "ELIBMAX": 132,
 "ELIBEXEC": 133,
 "ENOSYS": 52,
 "ENOTEMPTY": 55,
 "ENAMETOOLONG": 37,
 "ELOOP": 32,
 "EOPNOTSUPP": 138,
 "EPFNOSUPPORT": 139,
 "ECONNRESET": 15,
 "ENOBUFS": 42,
 "EAFNOSUPPORT": 5,
 "EPROTOTYPE": 67,
 "ENOTSOCK": 57,
 "ENOPROTOOPT": 50,
 "ESHUTDOWN": 140,
 "ECONNREFUSED": 14,
 "EADDRINUSE": 3,
 "ECONNABORTED": 13,
 "ENETUNREACH": 40,
 "ENETDOWN": 38,
 "ETIMEDOUT": 73,
 "EHOSTDOWN": 142,
 "EHOSTUNREACH": 23,
 "EINPROGRESS": 26,
 "EALREADY": 7,
 "EDESTADDRREQ": 17,
 "EMSGSIZE": 35,
 "EPROTONOSUPPORT": 66,
 "ESOCKTNOSUPPORT": 137,
 "EADDRNOTAVAIL": 4,
 "ENETRESET": 39,
 "EISCONN": 30,
 "ENOTCONN": 53,
 "ETOOMANYREFS": 141,
 "EUSERS": 136,
 "EDQUOT": 19,
 "ESTALE": 72,
 "ENOTSUP": 138,
 "ENOMEDIUM": 148,
 "EILSEQ": 25,
 "EOVERFLOW": 61,
 "ECANCELED": 11,
 "ENOTRECOVERABLE": 56,
 "EOWNERDEAD": 62,
 "ESTRPIPE": 135
};

Module["requestFullscreen"] = function Module_requestFullscreen(lockPointer, resizeCanvas) {
 Browser.requestFullscreen(lockPointer, resizeCanvas);
};

Module["requestFullScreen"] = function Module_requestFullScreen() {
 Browser.requestFullScreen();
};

Module["requestAnimationFrame"] = function Module_requestAnimationFrame(func) {
 Browser.requestAnimationFrame(func);
};

Module["setCanvasSize"] = function Module_setCanvasSize(width, height, noUpdates) {
 Browser.setCanvasSize(width, height, noUpdates);
};

Module["pauseMainLoop"] = function Module_pauseMainLoop() {
 Browser.mainLoop.pause();
};

Module["resumeMainLoop"] = function Module_resumeMainLoop() {
 Browser.mainLoop.resume();
};

Module["getUserMedia"] = function Module_getUserMedia() {
 Browser.getUserMedia();
};

Module["createContext"] = function Module_createContext(canvas, useWebGL, setInModule, webGLContextAttributes) {
 return Browser.createContext(canvas, useWebGL, setInModule, webGLContextAttributes);
};

var GLctx;

var ASSERTIONS = true;

function intArrayFromString(stringy, dontAddNull, length) {
 var len = length > 0 ? length : lengthBytesUTF8(stringy) + 1;
 var u8array = new Array(len);
 var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
 if (dontAddNull) u8array.length = numBytesWritten;
 return u8array;
}

function intArrayToString(array) {
 var ret = [];
 for (var i = 0; i < array.length; i++) {
  var chr = array[i];
  if (chr > 255) {
   if (ASSERTIONS) {
    assert(false, "Character code " + chr + " (" + String.fromCharCode(chr) + ")  at offset " + i + " not in 0x00-0xFF.");
   }
   chr &= 255;
  }
  ret.push(String.fromCharCode(chr));
 }
 return ret.join("");
}

var decodeBase64 = typeof atob === "function" ? atob : function(input) {
 var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
 var output = "";
 var chr1, chr2, chr3;
 var enc1, enc2, enc3, enc4;
 var i = 0;
 input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");
 do {
  enc1 = keyStr.indexOf(input.charAt(i++));
  enc2 = keyStr.indexOf(input.charAt(i++));
  enc3 = keyStr.indexOf(input.charAt(i++));
  enc4 = keyStr.indexOf(input.charAt(i++));
  chr1 = enc1 << 2 | enc2 >> 4;
  chr2 = (enc2 & 15) << 4 | enc3 >> 2;
  chr3 = (enc3 & 3) << 6 | enc4;
  output = output + String.fromCharCode(chr1);
  if (enc3 !== 64) {
   output = output + String.fromCharCode(chr2);
  }
  if (enc4 !== 64) {
   output = output + String.fromCharCode(chr3);
  }
 } while (i < input.length);
 return output;
};

function intArrayFromBase64(s) {
 if (typeof ENVIRONMENT_IS_NODE === "boolean" && ENVIRONMENT_IS_NODE) {
  var buf = Buffer.from(s, "base64");
  return new Uint8Array(buf["buffer"], buf["byteOffset"], buf["byteLength"]);
 }
 try {
  var decoded = decodeBase64(s);
  var bytes = new Uint8Array(decoded.length);
  for (var i = 0; i < decoded.length; ++i) {
   bytes[i] = decoded.charCodeAt(i);
  }
  return bytes;
 } catch (_) {
  throw new Error("Converting base64 string to bytes failed.");
 }
}

function tryParseAsDataURI(filename) {
 if (!isDataURI(filename)) {
  return;
 }
 return intArrayFromBase64(filename.slice(dataURIPrefix.length));
}

var asmLibraryArg = {
 "Mix_AllocateChannels": _Mix_AllocateChannels,
 "Mix_FreeChunk": _Mix_FreeChunk,
 "Mix_HaltChannel": _Mix_HaltChannel,
 "Mix_LoadWAV": _Mix_LoadWAV,
 "Mix_OpenAudio": _Mix_OpenAudio,
 "Mix_PlayChannelTimed": _Mix_PlayChannelTimed,
 "Mix_Playing": _Mix_Playing,
 "Mix_QuickLoad_RAW": _Mix_QuickLoad_RAW,
 "Mix_Volume": _Mix_Volume,
 "SDL_CreateRGBSurface": _SDL_CreateRGBSurface,
 "SDL_Delay": _SDL_Delay,
 "SDL_EnableKeyRepeat": _SDL_EnableKeyRepeat,
 "SDL_EnableUNICODE": _SDL_EnableUNICODE,
 "SDL_FreeSurface": _SDL_FreeSurface,
 "SDL_GetAppState": _SDL_GetAppState,
 "SDL_GetError": _SDL_GetError,
 "SDL_GetKeyboardState": _SDL_GetKeyboardState,
 "SDL_GetTicks": _SDL_GetTicks,
 "SDL_Init": _SDL_Init,
 "SDL_ListModes": _SDL_ListModes,
 "SDL_LockSurface": _SDL_LockSurface,
 "SDL_PollEvent": _SDL_PollEvent,
 "SDL_Quit": _SDL_Quit,
 "SDL_SetVideoMode": _SDL_SetVideoMode,
 "SDL_ShowCursor": _SDL_ShowCursor,
 "SDL_UnlockSurface": _SDL_UnlockSurface,
 "SDL_UpdateRect": _SDL_UpdateRect,
 "SDL_UpperBlit": _SDL_UpperBlit,
 "SDL_WM_GrabInput": _SDL_WM_GrabInput,
 "SDL_WM_SetCaption": _SDL_WM_SetCaption,
 "SDL_WarpMouse": _SDL_WarpMouse,
 "SDL_mutexP": _SDL_mutexP,
 "SDL_mutexV": _SDL_mutexV,
 "__assert_fail": ___assert_fail,
 "__call_sighandler": ___call_sighandler,
 "__cxa_allocate_exception": ___cxa_allocate_exception,
 "__cxa_atexit": ___cxa_atexit,
 "__cxa_begin_catch": ___cxa_begin_catch,
 "__cxa_end_catch": ___cxa_end_catch,
 "__cxa_find_matching_catch_2": ___cxa_find_matching_catch_2,
 "__cxa_find_matching_catch_3": ___cxa_find_matching_catch_3,
 "__cxa_free_exception": ___cxa_free_exception,
 "__cxa_rethrow": ___cxa_rethrow,
 "__cxa_throw": ___cxa_throw,
 "__cxa_uncaught_exceptions": ___cxa_uncaught_exceptions,
 "__localtime_r": ___localtime_r,
 "__resumeException": ___resumeException,
 "__sys_chdir": ___sys_chdir,
 "__sys_dup": ___sys_dup,
 "__sys_fcntl64": ___sys_fcntl64,
 "__sys_fstat64": ___sys_fstat64,
 "__sys_ftruncate64": ___sys_ftruncate64,
 "__sys_getcwd": ___sys_getcwd,
 "__sys_getdents64": ___sys_getdents64,
 "__sys_getpid": ___sys_getpid,
 "__sys_ioctl": ___sys_ioctl,
 "__sys_mkdir": ___sys_mkdir,
 "__sys_mmap2": ___sys_mmap2,
 "__sys_munmap": ___sys_munmap,
 "__sys_open": ___sys_open,
 "__sys_prlimit64": ___sys_prlimit64,
 "__sys_recvfrom": ___sys_recvfrom,
 "__sys_rename": ___sys_rename,
 "__sys_rmdir": ___sys_rmdir,
 "__sys_sendto": ___sys_sendto,
 "__sys_setrlimit": ___sys_setrlimit,
 "__sys_shutdown": ___sys_shutdown,
 "__sys_stat64": ___sys_stat64,
 "__sys_ugetrlimit": ___sys_ugetrlimit,
 "__sys_uname": ___sys_uname,
 "__sys_unlink": ___sys_unlink,
 "_dlopen_js": __dlopen_js,
 "_dlsym_js": __dlsym_js,
 "abort": _abort,
 "atexit": _atexit,
 "emscripten_asm_const_int": _emscripten_asm_const_int,
 "emscripten_builtin_mmap2": _emscripten_builtin_mmap2,
 "emscripten_builtin_munmap": _emscripten_builtin_munmap,
 "emscripten_cancel_main_loop": _emscripten_cancel_main_loop,
 "emscripten_get_heap_max": _emscripten_get_heap_max,
 "emscripten_get_module_name": _emscripten_get_module_name,
 "emscripten_pc_get_column": _emscripten_pc_get_column,
 "emscripten_pc_get_file": _emscripten_pc_get_file,
 "emscripten_pc_get_function": _emscripten_pc_get_function,
 "emscripten_pc_get_line": _emscripten_pc_get_line,
 "emscripten_resize_heap": _emscripten_resize_heap,
 "emscripten_return_address": _emscripten_return_address,
 "emscripten_set_main_loop": _emscripten_set_main_loop,
 "emscripten_stack_snapshot": _emscripten_stack_snapshot,
 "emscripten_stack_unwind_buffer": _emscripten_stack_unwind_buffer,
 "emscripten_thread_sleep": _emscripten_thread_sleep,
 "environ_get": _environ_get,
 "environ_sizes_get": _environ_sizes_get,
 "exit": _exit,
 "fd_close": _fd_close,
 "fd_fdstat_get": _fd_fdstat_get,
 "fd_read": _fd_read,
 "fd_seek": _fd_seek,
 "fd_write": _fd_write,
 "getTempRet0": _getTempRet0,
 "gettimeofday": _gettimeofday,
 "invoke_diii": invoke_diii,
 "invoke_fiii": invoke_fiii,
 "invoke_i": invoke_i,
 "invoke_ii": invoke_ii,
 "invoke_iii": invoke_iii,
 "invoke_iiii": invoke_iiii,
 "invoke_iiiii": invoke_iiiii,
 "invoke_iiiiid": invoke_iiiiid,
 "invoke_iiiiii": invoke_iiiiii,
 "invoke_iiiiiii": invoke_iiiiiii,
 "invoke_iiiiiiii": invoke_iiiiiiii,
 "invoke_iiiiiiiiiii": invoke_iiiiiiiiiii,
 "invoke_iiiiiiiiiiii": invoke_iiiiiiiiiiii,
 "invoke_iiiiiiiiiiiii": invoke_iiiiiiiiiiiii,
 "invoke_jiiii": invoke_jiiii,
 "invoke_v": invoke_v,
 "invoke_vi": invoke_vi,
 "invoke_vii": invoke_vii,
 "invoke_viii": invoke_viii,
 "invoke_viiii": invoke_viiii,
 "invoke_viiiiiii": invoke_viiiiiii,
 "invoke_viiiiiiiiii": invoke_viiiiiiiiii,
 "invoke_viiiiiiiiiiiiiii": invoke_viiiiiiiiiiiiiii,
 "invoke_viijii": invoke_viijii,
 "proc_exit": _proc_exit,
 "setTempRet0": _setTempRet0,
 "strftime": _strftime,
 "strftime_l": _strftime_l,
 "time": _time
};

var asm = createWasm();

var ___wasm_call_ctors = Module["___wasm_call_ctors"] = createExportWrapper("__wasm_call_ctors");

var _main = Module["_main"] = createExportWrapper("main");

var ___errno_location = Module["___errno_location"] = createExportWrapper("__errno_location");

var _htons = Module["_htons"] = createExportWrapper("htons");

var _memcpy = Module["_memcpy"] = createExportWrapper("memcpy");

var _memset = Module["_memset"] = createExportWrapper("memset");

var _malloc = Module["_malloc"] = createExportWrapper("malloc");

var _free = Module["_free"] = createExportWrapper("free");

var _ntohs = Module["_ntohs"] = createExportWrapper("ntohs");

var _fflush = Module["_fflush"] = createExportWrapper("fflush");

var _emscripten_builtin_malloc = Module["_emscripten_builtin_malloc"] = createExportWrapper("emscripten_builtin_malloc");

var ___dl_seterr = Module["___dl_seterr"] = createExportWrapper("__dl_seterr");

var __get_tzname = Module["__get_tzname"] = createExportWrapper("_get_tzname");

var __get_daylight = Module["__get_daylight"] = createExportWrapper("_get_daylight");

var __get_timezone = Module["__get_timezone"] = createExportWrapper("_get_timezone");

var stackSave = Module["stackSave"] = createExportWrapper("stackSave");

var stackRestore = Module["stackRestore"] = createExportWrapper("stackRestore");

var stackAlloc = Module["stackAlloc"] = createExportWrapper("stackAlloc");

var _emscripten_stack_init = Module["_emscripten_stack_init"] = function() {
 return (_emscripten_stack_init = Module["_emscripten_stack_init"] = Module["asm"]["emscripten_stack_init"]).apply(null, arguments);
};

var _emscripten_stack_get_free = Module["_emscripten_stack_get_free"] = function() {
 return (_emscripten_stack_get_free = Module["_emscripten_stack_get_free"] = Module["asm"]["emscripten_stack_get_free"]).apply(null, arguments);
};

var _emscripten_stack_get_end = Module["_emscripten_stack_get_end"] = function() {
 return (_emscripten_stack_get_end = Module["_emscripten_stack_get_end"] = Module["asm"]["emscripten_stack_get_end"]).apply(null, arguments);
};

var _setThrew = Module["_setThrew"] = createExportWrapper("setThrew");

var ___cxa_can_catch = Module["___cxa_can_catch"] = createExportWrapper("__cxa_can_catch");

var ___cxa_is_pointer_type = Module["___cxa_is_pointer_type"] = createExportWrapper("__cxa_is_pointer_type");

var _memalign = Module["_memalign"] = createExportWrapper("memalign");

var _emscripten_builtin_free = Module["_emscripten_builtin_free"] = createExportWrapper("emscripten_builtin_free");

var _emscripten_builtin_memalign = Module["_emscripten_builtin_memalign"] = createExportWrapper("emscripten_builtin_memalign");

var _emscripten_builtin_memset = Module["_emscripten_builtin_memset"] = createExportWrapper("emscripten_builtin_memset");

var __ZN6__asan9FakeStack17AddrIsInFakeStackEm = Module["__ZN6__asan9FakeStack17AddrIsInFakeStackEm"] = createExportWrapper("_ZN6__asan9FakeStack17AddrIsInFakeStackEm");

var __ZN6__asan9FakeStack8AllocateEmmm = Module["__ZN6__asan9FakeStack8AllocateEmmm"] = createExportWrapper("_ZN6__asan9FakeStack8AllocateEmmm");

var _asan_c_load_1 = Module["_asan_c_load_1"] = createExportWrapper("asan_c_load_1");

var _asan_c_load_1u = Module["_asan_c_load_1u"] = createExportWrapper("asan_c_load_1u");

var _asan_c_load_2 = Module["_asan_c_load_2"] = createExportWrapper("asan_c_load_2");

var _asan_c_load_2u = Module["_asan_c_load_2u"] = createExportWrapper("asan_c_load_2u");

var _asan_c_load_4 = Module["_asan_c_load_4"] = createExportWrapper("asan_c_load_4");

var _asan_c_load_4u = Module["_asan_c_load_4u"] = createExportWrapper("asan_c_load_4u");

var _asan_c_load_f = Module["_asan_c_load_f"] = createExportWrapper("asan_c_load_f");

var _asan_c_load_d = Module["_asan_c_load_d"] = createExportWrapper("asan_c_load_d");

var _asan_c_store_1 = Module["_asan_c_store_1"] = createExportWrapper("asan_c_store_1");

var _asan_c_store_1u = Module["_asan_c_store_1u"] = createExportWrapper("asan_c_store_1u");

var _asan_c_store_2 = Module["_asan_c_store_2"] = createExportWrapper("asan_c_store_2");

var _asan_c_store_2u = Module["_asan_c_store_2u"] = createExportWrapper("asan_c_store_2u");

var _asan_c_store_4 = Module["_asan_c_store_4"] = createExportWrapper("asan_c_store_4");

var _asan_c_store_4u = Module["_asan_c_store_4u"] = createExportWrapper("asan_c_store_4u");

var _asan_c_store_f = Module["_asan_c_store_f"] = createExportWrapper("asan_c_store_f");

var _asan_c_store_d = Module["_asan_c_store_d"] = createExportWrapper("asan_c_store_d");

var dynCall_j = Module["dynCall_j"] = createExportWrapper("dynCall_j");

var dynCall_jiji = Module["dynCall_jiji"] = createExportWrapper("dynCall_jiji");

var dynCall_viijii = Module["dynCall_viijii"] = createExportWrapper("dynCall_viijii");

var dynCall_jiiii = Module["dynCall_jiiii"] = createExportWrapper("dynCall_jiiii");

var dynCall_iiiiij = Module["dynCall_iiiiij"] = createExportWrapper("dynCall_iiiiij");

var dynCall_iiiiijj = Module["dynCall_iiiiijj"] = createExportWrapper("dynCall_iiiiijj");

var dynCall_iiiiiijj = Module["dynCall_iiiiiijj"] = createExportWrapper("dynCall_iiiiiijj");

var dynCall_viiijj = Module["dynCall_viiijj"] = createExportWrapper("dynCall_viiijj");

var dynCall_jii = Module["dynCall_jii"] = createExportWrapper("dynCall_jii");

var ___heap_base = Module["___heap_base"] = 279548096;

var ___global_base = Module["___global_base"] = 268435456;

function invoke_vi(index, a1) {
 var sp = stackSave();
 try {
  wasmTable.get(index)(a1);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iii(index, a1, a2) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_ii(index, a1) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_vii(index, a1, a2) {
 var sp = stackSave();
 try {
  wasmTable.get(index)(a1, a2);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiiiiii(index, a1, a2, a3, a4, a5, a6, a7) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4, a5, a6, a7);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_v(index) {
 var sp = stackSave();
 try {
  wasmTable.get(index)();
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiiiiiiiii(index, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiii(index, a1, a2, a3, a4) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_viiii(index, a1, a2, a3, a4) {
 var sp = stackSave();
 try {
  wasmTable.get(index)(a1, a2, a3, a4);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiiiiiiiiiii(index, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_fiii(index, a1, a2, a3) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_diii(index, a1, a2, a3) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_i(index) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)();
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiii(index, a1, a2, a3) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_viiiiiii(index, a1, a2, a3, a4, a5, a6, a7) {
 var sp = stackSave();
 try {
  wasmTable.get(index)(a1, a2, a3, a4, a5, a6, a7);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiiiii(index, a1, a2, a3, a4, a5, a6) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4, a5, a6);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiiiiiiiiii(index, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_viiiiiiiiii(index, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {
 var sp = stackSave();
 try {
  wasmTable.get(index)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_viii(index, a1, a2, a3) {
 var sp = stackSave();
 try {
  wasmTable.get(index)(a1, a2, a3);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_viiiiiiiiiiiiiii(index, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) {
 var sp = stackSave();
 try {
  wasmTable.get(index)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiiii(index, a1, a2, a3, a4, a5) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4, a5);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_iiiiid(index, a1, a2, a3, a4, a5) {
 var sp = stackSave();
 try {
  return wasmTable.get(index)(a1, a2, a3, a4, a5);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_jiiii(index, a1, a2, a3, a4) {
 var sp = stackSave();
 try {
  return dynCall_jiiii(index, a1, a2, a3, a4);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

function invoke_viijii(index, a1, a2, a3, a4, a5, a6) {
 var sp = stackSave();
 try {
  dynCall_viijii(index, a1, a2, a3, a4, a5, a6);
 } catch (e) {
  stackRestore(sp);
  if (e !== e + 0 && e !== "longjmp") throw e;
  _setThrew(1, 0);
 }
}

if (!Object.getOwnPropertyDescriptor(Module, "intArrayFromString")) Module["intArrayFromString"] = function() {
 abort("'intArrayFromString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "intArrayToString")) Module["intArrayToString"] = function() {
 abort("'intArrayToString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "ccall")) Module["ccall"] = function() {
 abort("'ccall' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "cwrap")) Module["cwrap"] = function() {
 abort("'cwrap' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setValue")) Module["setValue"] = function() {
 abort("'setValue' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getValue")) Module["getValue"] = function() {
 abort("'getValue' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "allocate")) Module["allocate"] = function() {
 abort("'allocate' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "UTF8ArrayToString")) Module["UTF8ArrayToString"] = function() {
 abort("'UTF8ArrayToString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "UTF8ToString")) Module["UTF8ToString"] = function() {
 abort("'UTF8ToString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF8Array")) Module["stringToUTF8Array"] = function() {
 abort("'stringToUTF8Array' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF8")) Module["stringToUTF8"] = function() {
 abort("'stringToUTF8' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF8")) Module["lengthBytesUTF8"] = function() {
 abort("'lengthBytesUTF8' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stackTrace")) Module["stackTrace"] = function() {
 abort("'stackTrace' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "addOnPreRun")) Module["addOnPreRun"] = function() {
 abort("'addOnPreRun' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "addOnInit")) Module["addOnInit"] = function() {
 abort("'addOnInit' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "addOnPreMain")) Module["addOnPreMain"] = function() {
 abort("'addOnPreMain' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "addOnExit")) Module["addOnExit"] = function() {
 abort("'addOnExit' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "addOnPostRun")) Module["addOnPostRun"] = function() {
 abort("'addOnPostRun' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeStringToMemory")) Module["writeStringToMemory"] = function() {
 abort("'writeStringToMemory' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeArrayToMemory")) Module["writeArrayToMemory"] = function() {
 abort("'writeArrayToMemory' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeAsciiToMemory")) Module["writeAsciiToMemory"] = function() {
 abort("'writeAsciiToMemory' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

Module["addRunDependency"] = addRunDependency;

Module["removeRunDependency"] = removeRunDependency;

if (!Object.getOwnPropertyDescriptor(Module, "FS_createFolder")) Module["FS_createFolder"] = function() {
 abort("'FS_createFolder' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

Module["FS_createPath"] = FS.createPath;

Module["FS_createDataFile"] = FS.createDataFile;

Module["FS_createPreloadedFile"] = FS.createPreloadedFile;

Module["FS_createLazyFile"] = FS.createLazyFile;

if (!Object.getOwnPropertyDescriptor(Module, "FS_createLink")) Module["FS_createLink"] = function() {
 abort("'FS_createLink' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

Module["FS_createDevice"] = FS.createDevice;

Module["FS_unlink"] = FS.unlink;

if (!Object.getOwnPropertyDescriptor(Module, "getLEB")) Module["getLEB"] = function() {
 abort("'getLEB' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getFunctionTables")) Module["getFunctionTables"] = function() {
 abort("'getFunctionTables' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "alignFunctionTables")) Module["alignFunctionTables"] = function() {
 abort("'alignFunctionTables' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerFunctions")) Module["registerFunctions"] = function() {
 abort("'registerFunctions' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "addFunction")) Module["addFunction"] = function() {
 abort("'addFunction' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "removeFunction")) Module["removeFunction"] = function() {
 abort("'removeFunction' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getFuncWrapper")) Module["getFuncWrapper"] = function() {
 abort("'getFuncWrapper' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "prettyPrint")) Module["prettyPrint"] = function() {
 abort("'prettyPrint' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "dynCall")) Module["dynCall"] = function() {
 abort("'dynCall' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getCompilerSetting")) Module["getCompilerSetting"] = function() {
 abort("'getCompilerSetting' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "print")) Module["print"] = function() {
 abort("'print' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "printErr")) Module["printErr"] = function() {
 abort("'printErr' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getTempRet0")) Module["getTempRet0"] = function() {
 abort("'getTempRet0' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setTempRet0")) Module["setTempRet0"] = function() {
 abort("'setTempRet0' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "callMain")) Module["callMain"] = function() {
 abort("'callMain' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "abort")) Module["abort"] = function() {
 abort("'abort' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "keepRuntimeAlive")) Module["keepRuntimeAlive"] = function() {
 abort("'keepRuntimeAlive' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "WasmOffsetConverter")) Module["WasmOffsetConverter"] = function() {
 abort("'WasmOffsetConverter' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "zeroMemory")) Module["zeroMemory"] = function() {
 abort("'zeroMemory' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stringToNewUTF8")) Module["stringToNewUTF8"] = function() {
 abort("'stringToNewUTF8' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setFileTime")) Module["setFileTime"] = function() {
 abort("'setFileTime' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "emscripten_realloc_buffer")) Module["emscripten_realloc_buffer"] = function() {
 abort("'emscripten_realloc_buffer' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "ENV")) Module["ENV"] = function() {
 abort("'ENV' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "ERRNO_CODES")) Module["ERRNO_CODES"] = function() {
 abort("'ERRNO_CODES' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "ERRNO_MESSAGES")) Module["ERRNO_MESSAGES"] = function() {
 abort("'ERRNO_MESSAGES' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setErrNo")) Module["setErrNo"] = function() {
 abort("'setErrNo' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "inetPton4")) Module["inetPton4"] = function() {
 abort("'inetPton4' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "inetNtop4")) Module["inetNtop4"] = function() {
 abort("'inetNtop4' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "inetPton6")) Module["inetPton6"] = function() {
 abort("'inetPton6' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "inetNtop6")) Module["inetNtop6"] = function() {
 abort("'inetNtop6' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "readSockaddr")) Module["readSockaddr"] = function() {
 abort("'readSockaddr' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeSockaddr")) Module["writeSockaddr"] = function() {
 abort("'writeSockaddr' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "DNS")) Module["DNS"] = function() {
 abort("'DNS' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getHostByName")) Module["getHostByName"] = function() {
 abort("'getHostByName' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "GAI_ERRNO_MESSAGES")) Module["GAI_ERRNO_MESSAGES"] = function() {
 abort("'GAI_ERRNO_MESSAGES' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "Protocols")) Module["Protocols"] = function() {
 abort("'Protocols' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "Sockets")) Module["Sockets"] = function() {
 abort("'Sockets' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getRandomDevice")) Module["getRandomDevice"] = function() {
 abort("'getRandomDevice' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "traverseStack")) Module["traverseStack"] = function() {
 abort("'traverseStack' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "UNWIND_CACHE")) Module["UNWIND_CACHE"] = function() {
 abort("'UNWIND_CACHE' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "withBuiltinMalloc")) Module["withBuiltinMalloc"] = function() {
 abort("'withBuiltinMalloc' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "readAsmConstArgsArray")) Module["readAsmConstArgsArray"] = function() {
 abort("'readAsmConstArgsArray' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "readAsmConstArgs")) Module["readAsmConstArgs"] = function() {
 abort("'readAsmConstArgs' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "mainThreadEM_ASM")) Module["mainThreadEM_ASM"] = function() {
 abort("'mainThreadEM_ASM' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "jstoi_q")) Module["jstoi_q"] = function() {
 abort("'jstoi_q' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "jstoi_s")) Module["jstoi_s"] = function() {
 abort("'jstoi_s' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getExecutableName")) Module["getExecutableName"] = function() {
 abort("'getExecutableName' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "listenOnce")) Module["listenOnce"] = function() {
 abort("'listenOnce' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "autoResumeAudioContext")) Module["autoResumeAudioContext"] = function() {
 abort("'autoResumeAudioContext' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "dynCallLegacy")) Module["dynCallLegacy"] = function() {
 abort("'dynCallLegacy' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getDynCaller")) Module["getDynCaller"] = function() {
 abort("'getDynCaller' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "dynCall")) Module["dynCall"] = function() {
 abort("'dynCall' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "callRuntimeCallbacks")) Module["callRuntimeCallbacks"] = function() {
 abort("'callRuntimeCallbacks' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "handleException")) Module["handleException"] = function() {
 abort("'handleException' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "runtimeKeepalivePush")) Module["runtimeKeepalivePush"] = function() {
 abort("'runtimeKeepalivePush' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "runtimeKeepalivePop")) Module["runtimeKeepalivePop"] = function() {
 abort("'runtimeKeepalivePop' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "callUserCallback")) Module["callUserCallback"] = function() {
 abort("'callUserCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "maybeExit")) Module["maybeExit"] = function() {
 abort("'maybeExit' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "safeSetTimeout")) Module["safeSetTimeout"] = function() {
 abort("'safeSetTimeout' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "asmjsMangle")) Module["asmjsMangle"] = function() {
 abort("'asmjsMangle' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "asyncLoad")) Module["asyncLoad"] = function() {
 abort("'asyncLoad' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "alignMemory")) Module["alignMemory"] = function() {
 abort("'alignMemory' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "mmapAlloc")) Module["mmapAlloc"] = function() {
 abort("'mmapAlloc' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "reallyNegative")) Module["reallyNegative"] = function() {
 abort("'reallyNegative' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "unSign")) Module["unSign"] = function() {
 abort("'unSign' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "reSign")) Module["reSign"] = function() {
 abort("'reSign' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "formatString")) Module["formatString"] = function() {
 abort("'formatString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "PATH")) Module["PATH"] = function() {
 abort("'PATH' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "PATH_FS")) Module["PATH_FS"] = function() {
 abort("'PATH_FS' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "SYSCALLS")) Module["SYSCALLS"] = function() {
 abort("'SYSCALLS' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "syscallMmap2")) Module["syscallMmap2"] = function() {
 abort("'syscallMmap2' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "syscallMunmap")) Module["syscallMunmap"] = function() {
 abort("'syscallMunmap' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getSocketFromFD")) Module["getSocketFromFD"] = function() {
 abort("'getSocketFromFD' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getSocketAddress")) Module["getSocketAddress"] = function() {
 abort("'getSocketAddress' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "JSEvents")) Module["JSEvents"] = function() {
 abort("'JSEvents' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerKeyEventCallback")) Module["registerKeyEventCallback"] = function() {
 abort("'registerKeyEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "specialHTMLTargets")) Module["specialHTMLTargets"] = function() {
 abort("'specialHTMLTargets' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "maybeCStringToJsString")) Module["maybeCStringToJsString"] = function() {
 abort("'maybeCStringToJsString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "findEventTarget")) Module["findEventTarget"] = function() {
 abort("'findEventTarget' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "findCanvasEventTarget")) Module["findCanvasEventTarget"] = function() {
 abort("'findCanvasEventTarget' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getBoundingClientRect")) Module["getBoundingClientRect"] = function() {
 abort("'getBoundingClientRect' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillMouseEventData")) Module["fillMouseEventData"] = function() {
 abort("'fillMouseEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerMouseEventCallback")) Module["registerMouseEventCallback"] = function() {
 abort("'registerMouseEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerWheelEventCallback")) Module["registerWheelEventCallback"] = function() {
 abort("'registerWheelEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerUiEventCallback")) Module["registerUiEventCallback"] = function() {
 abort("'registerUiEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerFocusEventCallback")) Module["registerFocusEventCallback"] = function() {
 abort("'registerFocusEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillDeviceOrientationEventData")) Module["fillDeviceOrientationEventData"] = function() {
 abort("'fillDeviceOrientationEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerDeviceOrientationEventCallback")) Module["registerDeviceOrientationEventCallback"] = function() {
 abort("'registerDeviceOrientationEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillDeviceMotionEventData")) Module["fillDeviceMotionEventData"] = function() {
 abort("'fillDeviceMotionEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerDeviceMotionEventCallback")) Module["registerDeviceMotionEventCallback"] = function() {
 abort("'registerDeviceMotionEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "screenOrientation")) Module["screenOrientation"] = function() {
 abort("'screenOrientation' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillOrientationChangeEventData")) Module["fillOrientationChangeEventData"] = function() {
 abort("'fillOrientationChangeEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerOrientationChangeEventCallback")) Module["registerOrientationChangeEventCallback"] = function() {
 abort("'registerOrientationChangeEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillFullscreenChangeEventData")) Module["fillFullscreenChangeEventData"] = function() {
 abort("'fillFullscreenChangeEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerFullscreenChangeEventCallback")) Module["registerFullscreenChangeEventCallback"] = function() {
 abort("'registerFullscreenChangeEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerRestoreOldStyle")) Module["registerRestoreOldStyle"] = function() {
 abort("'registerRestoreOldStyle' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "hideEverythingExceptGivenElement")) Module["hideEverythingExceptGivenElement"] = function() {
 abort("'hideEverythingExceptGivenElement' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "restoreHiddenElements")) Module["restoreHiddenElements"] = function() {
 abort("'restoreHiddenElements' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setLetterbox")) Module["setLetterbox"] = function() {
 abort("'setLetterbox' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "currentFullscreenStrategy")) Module["currentFullscreenStrategy"] = function() {
 abort("'currentFullscreenStrategy' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "restoreOldWindowedStyle")) Module["restoreOldWindowedStyle"] = function() {
 abort("'restoreOldWindowedStyle' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "softFullscreenResizeWebGLRenderTarget")) Module["softFullscreenResizeWebGLRenderTarget"] = function() {
 abort("'softFullscreenResizeWebGLRenderTarget' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "doRequestFullscreen")) Module["doRequestFullscreen"] = function() {
 abort("'doRequestFullscreen' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillPointerlockChangeEventData")) Module["fillPointerlockChangeEventData"] = function() {
 abort("'fillPointerlockChangeEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerPointerlockChangeEventCallback")) Module["registerPointerlockChangeEventCallback"] = function() {
 abort("'registerPointerlockChangeEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerPointerlockErrorEventCallback")) Module["registerPointerlockErrorEventCallback"] = function() {
 abort("'registerPointerlockErrorEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "requestPointerLock")) Module["requestPointerLock"] = function() {
 abort("'requestPointerLock' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillVisibilityChangeEventData")) Module["fillVisibilityChangeEventData"] = function() {
 abort("'fillVisibilityChangeEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerVisibilityChangeEventCallback")) Module["registerVisibilityChangeEventCallback"] = function() {
 abort("'registerVisibilityChangeEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerTouchEventCallback")) Module["registerTouchEventCallback"] = function() {
 abort("'registerTouchEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillGamepadEventData")) Module["fillGamepadEventData"] = function() {
 abort("'fillGamepadEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerGamepadEventCallback")) Module["registerGamepadEventCallback"] = function() {
 abort("'registerGamepadEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerBeforeUnloadEventCallback")) Module["registerBeforeUnloadEventCallback"] = function() {
 abort("'registerBeforeUnloadEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "fillBatteryEventData")) Module["fillBatteryEventData"] = function() {
 abort("'fillBatteryEventData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "battery")) Module["battery"] = function() {
 abort("'battery' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "registerBatteryEventCallback")) Module["registerBatteryEventCallback"] = function() {
 abort("'registerBatteryEventCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setCanvasElementSize")) Module["setCanvasElementSize"] = function() {
 abort("'setCanvasElementSize' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getCanvasElementSize")) Module["getCanvasElementSize"] = function() {
 abort("'getCanvasElementSize' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setImmediateWrapped")) Module["setImmediateWrapped"] = function() {
 abort("'setImmediateWrapped' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "clearImmediateWrapped")) Module["clearImmediateWrapped"] = function() {
 abort("'clearImmediateWrapped' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "polyfillSetImmediate")) Module["polyfillSetImmediate"] = function() {
 abort("'polyfillSetImmediate' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "demangle")) Module["demangle"] = function() {
 abort("'demangle' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "demangleAll")) Module["demangleAll"] = function() {
 abort("'demangleAll' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "jsStackTrace")) Module["jsStackTrace"] = function() {
 abort("'jsStackTrace' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stackTrace")) Module["stackTrace"] = function() {
 abort("'stackTrace' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getEnvStrings")) Module["getEnvStrings"] = function() {
 abort("'getEnvStrings' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "checkWasiClock")) Module["checkWasiClock"] = function() {
 abort("'checkWasiClock' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToI64")) Module["writeI53ToI64"] = function() {
 abort("'writeI53ToI64' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToI64Clamped")) Module["writeI53ToI64Clamped"] = function() {
 abort("'writeI53ToI64Clamped' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToI64Signaling")) Module["writeI53ToI64Signaling"] = function() {
 abort("'writeI53ToI64Signaling' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToU64Clamped")) Module["writeI53ToU64Clamped"] = function() {
 abort("'writeI53ToU64Clamped' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeI53ToU64Signaling")) Module["writeI53ToU64Signaling"] = function() {
 abort("'writeI53ToU64Signaling' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "readI53FromI64")) Module["readI53FromI64"] = function() {
 abort("'readI53FromI64' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "readI53FromU64")) Module["readI53FromU64"] = function() {
 abort("'readI53FromU64' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "convertI32PairToI53")) Module["convertI32PairToI53"] = function() {
 abort("'convertI32PairToI53' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "convertU32PairToI53")) Module["convertU32PairToI53"] = function() {
 abort("'convertU32PairToI53' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "uncaughtExceptionCount")) Module["uncaughtExceptionCount"] = function() {
 abort("'uncaughtExceptionCount' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "exceptionLast")) Module["exceptionLast"] = function() {
 abort("'exceptionLast' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "exceptionCaught")) Module["exceptionCaught"] = function() {
 abort("'exceptionCaught' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "ExceptionInfo")) Module["ExceptionInfo"] = function() {
 abort("'ExceptionInfo' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "CatchInfo")) Module["CatchInfo"] = function() {
 abort("'CatchInfo' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "exception_addRef")) Module["exception_addRef"] = function() {
 abort("'exception_addRef' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "exception_decRef")) Module["exception_decRef"] = function() {
 abort("'exception_decRef' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "Browser")) Module["Browser"] = function() {
 abort("'Browser' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "funcWrappers")) Module["funcWrappers"] = function() {
 abort("'funcWrappers' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "getFuncWrapper")) Module["getFuncWrapper"] = function() {
 abort("'getFuncWrapper' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "setMainLoop")) Module["setMainLoop"] = function() {
 abort("'setMainLoop' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "wget")) Module["wget"] = function() {
 abort("'wget' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "FS")) Module["FS"] = function() {
 abort("'FS' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "MEMFS")) Module["MEMFS"] = function() {
 abort("'MEMFS' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "TTY")) Module["TTY"] = function() {
 abort("'TTY' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "PIPEFS")) Module["PIPEFS"] = function() {
 abort("'PIPEFS' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "SOCKFS")) Module["SOCKFS"] = function() {
 abort("'SOCKFS' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "_setNetworkCallback")) Module["_setNetworkCallback"] = function() {
 abort("'_setNetworkCallback' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "tempFixedLengthArray")) Module["tempFixedLengthArray"] = function() {
 abort("'tempFixedLengthArray' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "miniTempWebGLFloatBuffers")) Module["miniTempWebGLFloatBuffers"] = function() {
 abort("'miniTempWebGLFloatBuffers' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "heapObjectForWebGLType")) Module["heapObjectForWebGLType"] = function() {
 abort("'heapObjectForWebGLType' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "heapAccessShiftForWebGLHeap")) Module["heapAccessShiftForWebGLHeap"] = function() {
 abort("'heapAccessShiftForWebGLHeap' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "GL")) Module["GL"] = function() {
 abort("'GL' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGet")) Module["emscriptenWebGLGet"] = function() {
 abort("'emscriptenWebGLGet' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "computeUnpackAlignedImageSize")) Module["computeUnpackAlignedImageSize"] = function() {
 abort("'computeUnpackAlignedImageSize' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGetTexPixelData")) Module["emscriptenWebGLGetTexPixelData"] = function() {
 abort("'emscriptenWebGLGetTexPixelData' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGetUniform")) Module["emscriptenWebGLGetUniform"] = function() {
 abort("'emscriptenWebGLGetUniform' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "webglGetUniformLocation")) Module["webglGetUniformLocation"] = function() {
 abort("'webglGetUniformLocation' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "webglPrepareUniformLocationsBeforeFirstUse")) Module["webglPrepareUniformLocationsBeforeFirstUse"] = function() {
 abort("'webglPrepareUniformLocationsBeforeFirstUse' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "webglGetLeftBracePos")) Module["webglGetLeftBracePos"] = function() {
 abort("'webglGetLeftBracePos' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "emscriptenWebGLGetVertexAttrib")) Module["emscriptenWebGLGetVertexAttrib"] = function() {
 abort("'emscriptenWebGLGetVertexAttrib' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "writeGLArray")) Module["writeGLArray"] = function() {
 abort("'writeGLArray' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "AL")) Module["AL"] = function() {
 abort("'AL' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "SDL_unicode")) Module["SDL_unicode"] = function() {
 abort("'SDL_unicode' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "SDL_ttfContext")) Module["SDL_ttfContext"] = function() {
 abort("'SDL_ttfContext' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "SDL_audio")) Module["SDL_audio"] = function() {
 abort("'SDL_audio' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "SDL")) Module["SDL"] = function() {
 abort("'SDL' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "SDL_gfx")) Module["SDL_gfx"] = function() {
 abort("'SDL_gfx' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "GLUT")) Module["GLUT"] = function() {
 abort("'GLUT' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "EGL")) Module["EGL"] = function() {
 abort("'EGL' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "GLFW_Window")) Module["GLFW_Window"] = function() {
 abort("'GLFW_Window' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "GLFW")) Module["GLFW"] = function() {
 abort("'GLFW' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "GLEW")) Module["GLEW"] = function() {
 abort("'GLEW' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "IDBStore")) Module["IDBStore"] = function() {
 abort("'IDBStore' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "runAndAbortIfError")) Module["runAndAbortIfError"] = function() {
 abort("'runAndAbortIfError' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "warnOnce")) Module["warnOnce"] = function() {
 abort("'warnOnce' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stackSave")) Module["stackSave"] = function() {
 abort("'stackSave' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stackRestore")) Module["stackRestore"] = function() {
 abort("'stackRestore' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stackAlloc")) Module["stackAlloc"] = function() {
 abort("'stackAlloc' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "AsciiToString")) Module["AsciiToString"] = function() {
 abort("'AsciiToString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stringToAscii")) Module["stringToAscii"] = function() {
 abort("'stringToAscii' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "UTF16ToString")) Module["UTF16ToString"] = function() {
 abort("'UTF16ToString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF16")) Module["stringToUTF16"] = function() {
 abort("'stringToUTF16' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF16")) Module["lengthBytesUTF16"] = function() {
 abort("'lengthBytesUTF16' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "UTF32ToString")) Module["UTF32ToString"] = function() {
 abort("'UTF32ToString' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "stringToUTF32")) Module["stringToUTF32"] = function() {
 abort("'stringToUTF32' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "lengthBytesUTF32")) Module["lengthBytesUTF32"] = function() {
 abort("'lengthBytesUTF32' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "allocateUTF8")) Module["allocateUTF8"] = function() {
 abort("'allocateUTF8' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

if (!Object.getOwnPropertyDescriptor(Module, "allocateUTF8OnStack")) Module["allocateUTF8OnStack"] = function() {
 abort("'allocateUTF8OnStack' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
};

Module["writeStackCookie"] = writeStackCookie;

Module["checkStackCookie"] = checkStackCookie;

if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_NORMAL")) Object.defineProperty(Module, "ALLOC_NORMAL", {
 configurable: true,
 get: function() {
  abort("'ALLOC_NORMAL' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
 }
});

if (!Object.getOwnPropertyDescriptor(Module, "ALLOC_STACK")) Object.defineProperty(Module, "ALLOC_STACK", {
 configurable: true,
 get: function() {
  abort("'ALLOC_STACK' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)");
 }
});

var calledRun;

function ExitStatus(status) {
 this.name = "ExitStatus";
 this.message = "Program terminated with exit(" + status + ")";
 this.status = status;
}

var calledMain = false;

dependenciesFulfilled = function runCaller() {
 if (!calledRun) run();
 if (!calledRun) dependenciesFulfilled = runCaller;
};

function callMain(args) {
 assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on Module["onRuntimeInitialized"])');
 assert(__ATPRERUN__.length == 0, "cannot call main when preRun functions remain to be called");
 var entryFunction = Module["_main"];
 args = args || [];
 var argc = args.length + 1;
 var argv = stackAlloc((argc + 1) * 4);
 _asan_js_store_4(argv >> 2, allocateUTF8OnStack(thisProgram));
 for (var i = 1; i < argc; i++) {
  _asan_js_store_4((argv >> 2) + i, allocateUTF8OnStack(args[i - 1]));
 }
 _asan_js_store_4((argv >> 2) + argc, 0);
 try {
  var ret = entryFunction(argc, argv);
  exit(ret, true);
  return ret;
 } catch (e) {
  return handleException(e);
 } finally {
  calledMain = true;
 }
}

function stackCheckInit() {
 _emscripten_stack_init();
 writeStackCookie();
}

function run(args) {
 args = args || arguments_;
 if (runDependencies > 0) {
  return;
 }
 stackCheckInit();
 preRun();
 if (runDependencies > 0) {
  return;
 }
 function doRun() {
  if (calledRun) return;
  calledRun = true;
  Module["calledRun"] = true;
  if (ABORT) return;
  initRuntime();
  preMain();
  if (Module["onRuntimeInitialized"]) Module["onRuntimeInitialized"]();
  if (shouldRunNow) callMain(args);
  postRun();
 }
 if (Module["setStatus"]) {
  Module["setStatus"]("Running...");
  setTimeout(function() {
   setTimeout(function() {
    Module["setStatus"]("");
   }, 1);
   doRun();
  }, 1);
 } else {
  doRun();
 }
 checkStackCookie();
}

Module["run"] = run;

function exit(status, implicit) {
 EXITSTATUS = status;
 if (keepRuntimeAlive()) {
  if (!implicit) {
   var msg = "program exited (with status: " + status + "), but keepRuntimeAlive() is set (counter=" + runtimeKeepaliveCounter + ") due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)";
   err(msg);
  }
 } else {
  exitRuntime();
 }
 procExit(status);
}

function procExit(code) {
 EXITSTATUS = code;
 if (!keepRuntimeAlive()) {
  if (Module["onExit"]) Module["onExit"](code);
  ABORT = true;
 }
 quit_(code, new ExitStatus(code));
}

if (Module["preInit"]) {
 if (typeof Module["preInit"] == "function") Module["preInit"] = [ Module["preInit"] ];
 while (Module["preInit"].length > 0) {
  Module["preInit"].pop()();
 }
}

var shouldRunNow = true;

if (Module["noInitialRun"]) shouldRunNow = false;

run();
