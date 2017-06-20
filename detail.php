<!DOCTYPE html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="initial-scale=1, maximum-scale=1, user-scalable=no, width=device-width">
  <script src="js/jquery-3.0.0.min.js"></script>
  <script src="https://code.highcharts.com/highcharts.js"></script>
  <script src="https://code.highcharts.com/modules/exporting.js"></script>

  <script type="text/javascript">

    $(function () {
      var chart = new Highcharts.chart('line_chart', {
      chart: {
          type: 'line'
      },
      title:{
          text: null
      },
      xAxis: {
          categories: ['월요일', '화요일', '수요일', '목요일', '금요일', '토요일', '일요일']
      },
      yAxis: {
        title: {
          text: null
        }
      },
      plotOptions: {
          line: {
              dataLabels: {
                  enabled: true
              },
              enableMouseTracking: false
          }
      },
      series: [{
          name: '이용빈도',
          data: [2, 1, 3, 3, 4, 7, 6]
      }, {
          name: '예약빈도',
          data: [1, 2, 4, 3, 5, 8, 6]
      }]
  });
});
  </script>
  <title>RSMS</title>

  <style>
  body {
    width: 100%;
    overflow: auto;
  }

  #back-button-detail {
    width: 20px;
    height: 20px;
    margin-left: 10px;
    margin-bottom: 5px;
    float: left;
  }

  .detail {
    width: 100%;
    height: 100%;
    padding: 10px;
    overflow: auto;
  }

  .detail-img {
    width: 100%;
    height: 200px;
    margin: 10px;
  }

  .reserve_table {
    width: 95%;
    height: 150px;
    margin: 10px;
    margin-top: 20px;
  }

  th {
    height: 10px;
    text-align: center;
  }

  .week {
    width: 14%;
    height: 20px;
    text-align: center;
    border-right: 1px solid gray;
  }

  .week-end {
    width: 14%;
    height: 20px;
    text-align: center;
  }

  #map {
    width: 90%;
    height: 200px;
    margin: 10px;
    margin-left: 20px;
    margin-right: 20px;
  }

  #stadium-name {
    float: left;
    margin: 0 auto;
    margin-left: 10px;
    margin-right: 20px;
    font-size: 30px;
    vertical-align: middle;
    width: 100%;
  }

  .infomation {
    height: 30px;
    margin-top: 20px;
    margin-bottom: 20px;
  }

  .detail-location, .detail-use, .detail-text {
    height: 25px;
    float: left;
    margin-left: 10px;
    vertical-align: middle;
  }

  #detail-address {
    width: 80%;
    font-size: 20px;
    margin-top: 0px;
    margin-left: 10px;
  }

  #detail-usable {
    width: 70%;
    font-size: 20px;
    margin-top: 0px;
    margin-left: 10px;
  }

  .detail-reserve {
    float: right;
    margin-right: 20px;
    vertical-align: middle;
  }

  #confirmButton {
    float: right;
    margin-right: 20px;
    margin-bottom: 10px;
    vertical-align: middle;
  }
  select {
    display: inline-block;
    vertical-align: middle;
  }
  </style>
</head>

<body>
  <div class="detail">
    <img class="detail-img" src="img/seoul.png">
    <div class="infomation">
      <h6 id="stadium-name"></h6>
      <img class="detail-location" src="img/custominfo_location.png"><p class="detail-text" id="detail-address">서울시 송파구 송파2동</p>
      <span><img class="detail-use" src="img/custominfo_person.png" hspace="10"><p class="detail-text" id="detail-usable">사용인원</p>
      <img class="detail-reserve" src = "img/favorite.png" id="detail-reserve" onclick="document.getElementById('modal').style.display='block'"></span>
    </div>

    <table class="reserve_table">
      <tr>
        <th id="th_0"></th>
        <th id="th_1"></th>
        <th id="th_2"></th>
        <th id="th_3"></th>
        <th id="th_4"></th>
        <th id="th_5"></th>
        <th id="th_6"></th>
      </tr>
      <tr>
        <td class="week" id="week_0"></td>
        <td class="week" id="week_1"></td>
        <td class="week" id="week_2"></td>
        <td class="week" id="week_3"></td>
        <td class="week" id="week_4"></td>
        <td class="week" id="week_5"></td>
        <td class="week-end" id="week_6"></td>
      </tr>
    </table>

    <div id="modal" class="w3-modal">
      <div class="w3-modal-content w3-animate-opacity">
        <header class="w3-container">
        <span onclick="document.getElementById('modal').style.display='none'" class="w3-button w3-display-topright">&times;</span>
        <h3>예약정보 입력</h3>
        <div class="w3-container">
          <span>날짜 <select id="reserveDay">
            <option value="" selected="selected">예약날짜</option>
            <option id="reserveDay_0" value="0"></option>
            <option id="reserveDay_1" value="1"></option>
            <option id="reserveDay_2" value="2"></option>
            <option id="reserveDay_3" value="3"></option>
            <option id="reserveDay_4" value="4"></option>
            <option id="reserveDay_5" value="5"></option>
            <option id="reserveDay_6" value="6"></option>
          </select>
        </span>

        <span>
          시간 <select id="reserveTime">
            <option value="" selected="selected">예약시간</option>
            <option id="reserveTime_0" value="0">09:00</option>
            <option id="reserveTime_1" value="1">10:00</option>
            <option id="reserveTime_2" value="2">11:00</option>
            <option id="reserveTime_3" value="3">12:00</option>
            <option id="reserveTime_4" value="4">13:00</option>
            <option id="reserveTime_5" value="5">14:00</option>
            <option id="reserveTime_6" value="6">15:00</option>
          </select> </span>

          <span>
          이용시간 <select id="reserveDuration">
            <option value="" selected="selected">이용시간</option>
            <option id="reserveDuration_0" value="0">1시간</option>
            <option id="reserveDuration_1" value="1">2시간</option>
          </select></span>
        </div>

        <button class="w3-button w3-white w3-border w3-border-blue w3-round-large" id="confirmButton" onclick="confirmReserve()">예약하기</button>
      </div>
    </div>
    <div id="line_chart" style="min-width: 310px; height: 200px; margin: 0 auto"></div>
    <div id="map"></div>
  </div>

  <script type="text/javascript">
  var stadium_info, stadium_reserve;
  var icon = [ 'img/soccer_usable.png', 'img/soccer_unusable.png' ];

  //drawCharts();
  readData();
  readyDisplay();
  initDetail();

  //stadim_info = JSON.parse(stadium_reserve);

  function confirmReserve() {
    var today = new Date();
    var month = (today.getMonth() + 1);
    if(month < 10) {
      month = '0' + month;
    }
    var todayFormat = (1900 + today.getYear()) + '-' + month + '-';

    var day = document.getElementById('reserveDay').value;
    var date = today.getDate() + parseInt(day);
    todayFormat += date;
    console.log("selected Day : " + todayFormat);

    var time = document.getElementById('reserveTime').options[document.getElementById('reserveTime').selectedIndex].text;
    console.log("selected Time : " + time);

    var duration = parseInt(document.getElementById('reserveDuration').value) + 1;
    console.log("duration : " + duration);

    var user = localStorage.getItem('user');
    console.log("user : " + user);

    var stadium = localStorage.getItem('stadium');

    var week = ['일요일', '월요일', '화요일', '수요일', '목요일', '금요일', '토요일'];
    var weekDay = week[today.getDay()];

    var reserveId = $.ajax({
      type: 'POST',
      url: 'http://localhost/database/reserveConfirm.php',
      data: { user : user, day : weekDay, id : stadium, time : todayFormat, start : time, hour : duration  },
      crossDomain: true,
      cache: false,
      dataType: 'json',
      async: false,
      error: function(request, status, error) {
        alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
        //history.go(0);
      }
    }).responseText;

    console.log(JSON.parse(reserveId));

    localStorage.setItem('reserveNum', JSON.parse(reserveId));
    document.getElementById('modal').style.display='none';
  }

  function readData() {
    /* 사용자가 선택한 시설의 정보를 가져옴 */
    var number =  localStorage.getItem('stadium');
    console.log('stadium: ' + number);

    stadium_info = $.ajax({
      type: 'POST',
      url: 'http://localhost/database/readInfo.php',
      data: { stadium : number },
      crossDomain: true,
      cache: false,
      dataType: 'json',
      async: false,
      error : function(request,status,error) {
        alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
        //history.go(0);
      }
    }).responseText;

    stadium_info = JSON.parse(stadium_info);
    console.log("info: " + stadium_info);

    stadium_reserve = $.ajax({
      type: 'POST',
      url: 'http://localhost/database/readReservation.php',
      data: { stadium: number },
      crossDomain: true,
      cache: false,
      dataType: 'json',
      async: false,
      error : function(request,status,error) {
        alert("code:" + request.status + "\n" + "message:" + request.responseText + "\n" + "error:" + error);
        //history.go(0);
      }
    }).responseText;

    stadium_reserve = JSON.parse(stadium_reserve);
    console.log("reserve : " + stadium_reserve);
  } // readData

  function readyDisplay() {
    var week = ['일요일', '월요일', '화요일', '수요일', '목요일', '금요일', '토요일'];

    var today = new Date();
    console.log("today: " + today);
    //console.log("tomorrow: " + new Date(date.valueOf() + (24*60*60*1000)));
    var day = today.getDay();
    console.log("day : " + day) ;

    // 날짜 변환
    var year = 1900 + today.getYear();
    var month = today.getMonth() + 1;

    if(month < 10) {
      month = '0' + month;
    }
    var date = today.getDate();
    console.log("year : " + (1900 + today.getYear()));
    console.log("month : " + (today.getMonth() + 1));
    console.log("date+1 : " + (today.getDate() + 1));

    // 요일 추가
    var th, i, num;
    for(i=0; i<7; i++) {
      num = day + i;
      if(num < 7) {
        th = document.getElementById('th_'+i);
        th.innerHTML = week[num];

        reserveDay = document.getElementById('reserveDay_'+i);
        reserveDay.innerHTML = week[num];
      }
      else {
        num -= 7;
        th = document.getElementById('th_'+i);
        th.innerHTML = week[num];

        reserveDay = document.getElementById('reserveDay_'+i);
        reserveDay.innerHTML = week[num];
      }
    }

    var week;
    var string, timeList;
    var startTime, endTime;
    var time = year + '-' + month + '-' + date;

    console.log("Today : " + time);
    // 예약시간 추가
    var j = 0;
    console.log("reserve length : " + stadium_reserve.length);
    for(i=0; i<stadium_reserve.length; i++) {
      while(stadium_reserve[i][5] != time) {
        date += 1;
        time = year + '-' + month + '-' + date;
        j++;
      }
      console.log("j: " + j);

      if(stadium_reserve[i][5] === time) {
        startTime = stadium_reserve[i][6].substring(0, 2);
        endTime = parseInt(startTime) + parseInt(stadium_reserve[i][3]);
        endTime = endTime.toString() + ':00';

        timeList = stadium_reserve[i][6] + '-' + endTime + '<br>';

        week = document.getElementById('week_' + j);
        week.innerHTML += timeList;
        console.log(endTime);
      }
      else {
        j++;
        time = stadium_reserve[i][5];

        startTime = stadium_reserve[i][6].substring(0, 2);
        endTime = parseInt(startTime) + parseInt(stadium_reserve[i][3]);
        endTime = endTime.toString() + ':00';

        timeList = stadium_reserve[i][6] + '-' + endTime + '<br>';

        week = document.getElementById('week_' + j);
        week.innerHTML += timeList;
        console.log(endTime);
      }
    }
  } // readyDisplay

  function initDetail() {
    /* 사용자가 선택한 시설의 정보를 적절하게 배치하는 함수 */
    /* 지도 초기화 */
    console.log('lat : ' + parseFloat(stadium_info[0][5]) + ', lng : ' + parseFloat(stadium_info[0][6]));

    var myLatLng = {lat: parseFloat(stadium_info[0][5]), lng: parseFloat(stadium_info[0][6])};

    var map = new google.maps.Map(document.getElementById('map'), {
      zoom: 15,
      center: myLatLng
    });

    var marker = new google.maps.Marker({
      position: myLatLng,
      icon: icon[0],
      scaledSize: new google.maps.Size(1, 1),
      map: map
    });

    /* 정보 초기화 */
    document.getElementById('stadium-name').innerHTML = stadium_info[0][1];
    document.getElementById('detail-usable').innerHTML = stadium_info[0][7];
    document.getElementById('detail-address').innerHTML = stadium_info[0][2] + " " + stadium_info[0][3] + " " + stadium_info[0][4];
  } // initDetail

  function removeBack_detail() {
    /* 시설 정보 화면의 back-button을 삭제하는 함수 */
    var header = document.getElementById('Header');
    var backDetail = document.getElementById('back-button-detail');

    header.removeChild(backDetail);
  } // removeBack_detail

  function addBack() {
    /* 시설 목록으로 돌아갈 때에 back-button 삽입을 위한 함수 */
    var back = document.createElement('img');
    back.id = 'back-button';
    back.src = 'img/back_button.png';

    var header = document.getElementById('Header')
    header.insertBefore(back, header.childNodes[0]);
  } // addBack

  $('#back-button-detail').on('click', function(e) {
    /* 이전화면으로 돌아가는 버튼을 클릭했을 때에 발생하는 이벤트 */
    //alert('back-botton-detail click');

    $.ajax({
      type : 'GET',
      url : 'list.php',
      dataType : 'html',
      error : function() {
        history.go(0);
      },
      success : function(data) {
        removeBack_detail();
        addBack();
        $('#Content').html(data);
      }
    });
  });
  </script>
</body>
