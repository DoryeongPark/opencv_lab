<!DOCTYPE html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="initial-scale=1, maximum-scale=1, user-scalable=no, width=device-width">
  <title>RSMS</title>

  <style media="screen">
  li {
    list-style: none;
  }

  .community-list {
    padding-bottom: 54px;
  }

  .item {
    width: 80%;
    height: 120px;
    line-height: 120px;
    margin: 20px auto;
    margin-bottom: 10px !important;
    color: #f7b400;
    font-size: 20px;
    font-weight: bold;
    text-align: center;
    background-position: center;
    background-image: no-repeat;
    background-size: contain;
  }

  #back-button {
    width: 20px;
    height: 20px;
    margin-left: 10px;
    margin-bottom: 5px;
    float: left;
  }
  </style>
</head>

<body>
  <div class="community-list" id="community-list">
    <!-- <li>
      <div class="item" name="seoul">
        <img name="seoul">
        서울
      </div>
    </li>
    <li>
      <div class="item" id="gyeonggi-do">
        경기
      </div>
    </li> -->
  </div>

  <script type="text/javascript">
  addCommunity('seoul', '서울');
  addCommunity('gyeonggi-do', '경기도');

  function addBack() {
    /* back button 추가용 */
    var back = document.createElement('img');
    back.id = 'back-button';
    back.src = 'img/back_button.png';

    var header = document.getElementById('Header')
    header.insertBefore(back, header.childNodes[0]);
  }

  function addCommunity(name, name_) {
    /* 지역 정보를 이용하여 리스트를 생성하는 함수 */
    var item = document.createElement('div');
    item.id = name_;
    item.className = 'item';
    item.innerHTML = name_;

    var imageUrl = 'url(img/' + name + '.png)';
    item.style.backgroundImage = imageUrl;
    item.style.backgroundRepeat = 'no-repeat';

    var community = document.getElementById('community-list');
    community.appendChild(item);
  }

  /* 각 지역 아이템의 클릭이벤트 등록 */
  $('.item').on('click', function(e) {
    //alert(e.target.id); // 이걸 가지고 php로...

    /* 사용자가 선택한 지역의 정보를 localStorage에 저장하여 다음 페이지에 전달 */
    var dataString = e.target.id;
    localStorage.setItem('previous', 'community');
    localStorage.setItem('city', dataString);

    // 시설 목록을 보여주는 화면으로 이동
    $.ajax({
        type : "GET",
        url : "list.php",
        // error : function() {
        //   history.go(0);
        // },
        error:function(request,status,error){
          alert("code:"+request.status+"\n"+"message:"+request.responseText+"\n"+"error:"+error);
        },
        success : function(data) {
          addBack();
          $('#Content').html(data);
        }
      });
  });
  </script>
</body>
