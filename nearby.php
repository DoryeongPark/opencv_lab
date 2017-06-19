<!DOCTYPE html>
<head>
  <meta name="viewport" content="initial-scale=1.0, user-scalable=no">
  <meta charset="utf-8">

  <style media="screen">
  html, body {
    width: 100%;
    height: 100%;
    margin: 0 auto;
  }

  #map {
    height: 100%;
    padding-top: 55px;
    padding-bottom: 54px;
  }

  #infowindow {
    width: 180px;
    height: 60px;
  }

  .info-text {
    margin-left: 15px auto;
    font-size: 10px;
  }

  .info-img {
    margin: 5px;
    width: 12px;
    height: 12px;
  }

  #stadium-name {
    margin: 5px;
    margin-bottom: 10px !important;
    font-size: 18px;
    font-weight: bold;
  }

  .favorite {
    margin-top: 25px;
    margin-left: 10px;
    margin-right: 0px;
    width: 30px;
    height: 30px;
  }
  </style>
</head>

<body>
  <div id="map"></div>

  <script>
  var map, icon, infowindow, locations;
  var current_lng, current_lat;

  initMap();
  initPos();

  document.addEventListener('deviceready', onDeviceReady, false);

  function onDeviceReady() {
    // 사용자의 기기고유번호(UUID)를 얻어오는 함수
    localStorage.setItem('user', device.uuid);
    //alert(localStorage.getItem('user'));
  }

  function initMap() {
    /* 지도 초기화 */
    map = new google.maps.Map(document.getElementById('map'), {
      center: {lat: 37.5662952, lng: 126.9779451},
      zoom: 15
    });

    icon = [ ['img/soccer_usable.png', 'img/baseball_usable.png'],  ['img/soccer_unusable.png', 'img/baseball_unusable.png'] ];

    infowindow = new google.maps.InfoWindow({
      minWidth: 300,
      minHeight: 200
    });
  } // initMap();

  function initPos() {
    /* 지도의 중심을 현재 위치로 바꾸는 부분
       중심을 변경하고나면 현재 위치를 기준으로 반경 5Km 이내의 시설을 검색한다 */
    if (navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(function(position) {
        var pos = { // 현재 위치를 pos에 저장
          lat: position.coords.latitude,
          lng: position.coords.longitude
        };

        current_lat = pos.lat;
        current_lng = pos.lng;

        //console.log("initPos) lat : " + current_lat + ", lng: " + current_lng);
        console.log("initPos) lat : " + current_lat + ", lng: " + current_lng);

        map.setCenter(pos); // 현재 위치를 지도의 중심으로 지정

        readLocations();
      });
    } else { // 브라우저가 Geolocation을 지원하지않을 때
      handleLocationError(false, infoWindow, map.getCenter());
    }
  } // initPos

  function readLocations()
  {
    /* 현재 위치를 기준으로 반경 5Km 이내의 시설을 데이터베이스 내에서 검색하여
       결과를 가져오는 함수 */
    //console.log("readLocations) lat : " + current_lat + ", lng: " + current_lng);
    console.log("readLocations) lat : " + current_lat + ", lng: " + current_lng);

    locations = $.ajax({
      type: "POST",
      url:"http://localhost/database/readNearBy.php",
      data: { lat : current_lat, lng : current_lng },
      crossDomain: true,
      cache: false,
      dataType: 'json',
      async: false,
      error : function(request,status,error) {
        alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
        //history.go(0);
      }
    }).responseText;

    locations = JSON.parse(locations);
    console.log(locations[0]);

    registIcon();
  } // readLocations

  function handleLocationError(browserHasGeolocation, infoWindow, pos) {
    infoWindow.setPosition(pos);
    infoWindow.setContent(browserHasGeolocation ?
      'Error: The Geolocation service failed.' : 'Error: Your browser doesn\'t support geolocation.');
  } // handleLocationError

  function registIcon() {
    /* 주변 시설의 마커 등록 및 마커 클릭 시의 이벤트 등록 */
    var marker, col, col_, i;

    for (i = 0; i < locations.length; i++) {
      if(locations[i][8] == "축구장" || locations[i][8] == "간이운동장") col_ = 0;
      else if(locations[i][8] == "야구장") col_ = 1;

      if(locations[i][7] == "사용가능") {
        // Icon 설정을 위한 카테고리 확인
        col = 0;
      }
      else {
        col = 1;
      }

      console.log("col : " + col + ", col_ : " + col_);

      marker = new google.maps.Marker({
        position: new google.maps.LatLng(locations[i][5], locations[i][6]),
        icon: icon[col][col_],
        scaledSize: new google.maps.Size(1, 1), // scaled size for marker
        map: map
      });

      var contentString;
      var favorite;
      var status;
      google.maps.event.addListener(marker, 'click', (function(marker, i) {
        return function() {
          if(locations[i][6]) { // 사용자가 가예약을 한 시설일 경우 이미지
            favorite = "img/favorite.png";
          }
          else { // 사용자가 가예약을 하지않은 시설일 경우 이미지
            favorite = "img/users_inactive.png";
          }

          var status = false;
          contentString = '<div id="infowindow">' + '<img class="favorite" id="' + i + '" src="' + favorite + '" + onclick="favorite(' + i + ', ' + status + ')" align="right" hspace="0">' +
          '<h1 id="stadium-name">' + locations[i][1] + '</h1>' + '<span>' + '<img class="info-img" src="img/custominfo_location.png" style="float: left;" hspace="10">'
          + '<p class="info-text" id="stadium-address">' + locations[i][2] + ' ' + locations[i][3] + ' ' + locations[i][4] + '</p>' + '</span>' + '<span>' + '<img class="info-img" src="img/custominfo_person.png" style="float: left;" hspace="10">'
          +'<p class="info-text" id="stadium-status">' + locations[i][7] + '</p>' + '</span>' + '</div>';
          infowindow.setContent(contentString);
          infowindow.open(map, marker);
        }
      })(marker, i));
    } // end for loop
  } // registIcon

  function favorite(id, status) {
    /* 사용자 가예약에 따라 해당 시설의 가예약 이미지와 가예약 데이터를 변경한다 */
    var num = id;
    var id = document.getElementById(id);

    if(status) { // 가예약한 상태일 경우, 가예약하지않은 상태로 변경
      id.src = 'img/users_inactive.png';
      locations[num][7] = false;

      var num = localStorage.getItem('reserveNum');

      // 가예약 취소내역을 데이터베이스에 반영
      $.ajax({
        type: 'POST',
        url: 'http://localhost/database/userReserve.php',
        data: { reserveNum : num, action : 'cancel' },
        crossDomain: false,
        dataType: 'json',
        async: false,
        error : function(request,status,error) {
          alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
          //history.go(0);
        }
      });

      localStorage.setItem('reserveNum', 'null');
    }
    else { // 가예약하지않은 상태인 경우, 가예약 팝업창을 오픈
      if(confirm('예약하시겠습니까?') == true) {
          // 사용자가 가예약할 시, 세부정보 화면으로 이동
          $.ajax({
            type: 'GET',
            url: 'detail.php',
            data: { stadium : locations[num][0] },
            error:function(request,status,error){
              alert("code:"+request.status+"\n"+"message:"+request.responseText+"\n"+"error:"+error);
            },
            success : function(data) {
              $('#Content').html(data);
            }
          });
      }
      var stadium = id;
    }
  } // favorite
  </script>
</body>
