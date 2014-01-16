var visionCIApp=angular.module('visionCIApp', []);
var nextId=0;

function dist(a, b){
	var x=(a.x-b.x);
	var y=(a.y-b.y);
	return Math.sqrt(x*x+y*y);
}

visionCIApp.controller('TestsuiteController', function($scope, $http){
	$scope.selectedId=null;
	$http.get('/testsuite.json').then(function(data){
		$scope.testsuite=data.data;
		nextId=_.max($scope.testsuite, function(x){return x.id;}).id+1;
	});
	$scope.isPassingTest=function(testcase){
		if(!testcase.results){
			return false;
		}
		if(testcase.expected.length!==testcase.results.length){
			return false;
		}
		return _.every(
			testcase.expected,
			function(expected){
				return _.find(testcase.results, function(result){
					return (
						Math.abs(expected.radius-result.radius)<=expected.radiusError
						&& dist(expected.center, result.center)<=expected.centerError
					);
				});
			}
		);
	};
	$scope.isFailingTest=function(testcase){
		return testcase.results && !$scope.isPassingTest(testcase);
	};
	$scope.passFailTestClass=function(testcase){
		if(testcase.results){
			if($scope.isPassingTest(testcase)){
				return 'passing';
			}else{
				return 'failing';
			}
		}else{
			return "";
		}
	};
	$scope.select=function(testcase){
		$scope.selectedId=testcase?testcase.id:null;
	};
	$scope.isSelected=function(testcase){
		return $scope.selectedId===testcase.id;
	};
	$scope.selectedClass=function(testcase){
		return $scope.isSelected(testcase)?'selected':'';
	};
	$scope.expected_center_mouse_down=function(expected, event){
		event=$.event.fix(event);
		var x=event.pageX;
		var y=event.pageY;
		var errorMode=event.shiftKey;
		var elem=event.target;
		while(elem.tagName.toLowerCase()!='svg'){
			elem=elem.offsetParent;
		}
		var center=$(elem).offset();
		center={x:center.left, y:center.top};
		center.x+=expected.center.x;
		center.y+=expected.center.y;
		
		function mousemove(event){
			if(errorMode){
				expected.centerError-=dist({x:event.pageX, y:event.pageY}, center);
				expected.centerError=Math.abs(expected.centerError);
			}else{
				expected.center.x-=(x-event.pageX);
				expected.center.y-=(y-event.pageY);
				x=event.pageX;
				y=event.pageY;
			}
			setTimeout(_.bind($scope.$apply, $scope), 0);
		}
		$(document.body).on('mousemove', mousemove);
		$(document.body).one('mouseup', function(event){
			mousemove(event);
			$(document.body).off('mousemove', mousemove);
		});
	};
	$scope.expected_radius_mouse_down=function(expected, event){
		event=$.event.fix(event);
		var x=event.pageX;
		var y=event.pageY;
		var errorMode=event.shiftKey;
		var elem=event.target;
		while(elem.tagName.toLowerCase()!='svg'){
			elem=elem.offsetParent;
		}
		var center=$(elem).offset();
		center={x:center.left, y:center.top};
		center.x+=expected.center.x;
		center.y+=expected.center.y;
		
		function mousemove(event){
			if(errorMode){
				expected.radiusError=dist({x:event.pageX, y:event.pageY}, {x:x, y:y});
			}else{
				expected.radius=dist({x:event.pageX, y:event.pageY}, center);
			}
			setTimeout(_.bind($scope.$apply, $scope), 0);
		}
		$(document.body).on('mousemove', mousemove);
		$(document.body).one('mouseup', function(event){
			mousemove(event);
			$(document.body).off('mousemove', mousemove);
		});
	};
	$scope.remove=function(obj, arr){
		if(!confirm("Do you want to delete?")){
			return;
		}
		var idx=arr.indexOf(obj);
		if(idx>=0){
			arr.splice(idx, 1);
		}
	};
	$scope.runTests=function(){
		$('#status').text('Running...');
		$('#logo').contents().find('#bolt')[0].style.fill='white';
		var suite=_.map($scope.testsuite, _.identity);
		function func(){
			if(suite.length===0){
				var allPassing=_.every($scope.testsuite, $scope.isPassingTest);
				$('#status').text(allPassing?'All passing':'Some tests failed');
				$('#logo').contents().find('#bolt')[0].style.fill=allPassing?'green':'red';
			}else{
				var test=suite.pop();
				RunTest(test.url).then(function(results){
					test.results=results;
					func();
				});
			}
			setTimeout(_.bind($scope.$apply, $scope), 0);
		}
		func();
	};
	$scope.save=function(){
		$.ajax('/testsuite.json', {
			method:'post',
			contentType:'application/json',
			data:angular.toJson(_.map($scope.testsuite, function(testcase){
				testcase=_.clone(testcase);
				delete testcase.results;
				return testcase;
			}))
		}).done(function(){
			alert("Saved!");
		});
	};
	$('#file-upload').change(function(){
		var fu=$('#file-upload')[0];
		if(
			fu.value==="" || 
			(
				fu.value.indexOf('.jpg')<0  &&
				fu.value.indexOf('.jpeg')<0
			)
			/*it really ought to be an extension, but this check should suffice*/
		){
			fu.value="";
			return;
		}
		var f=fu.files[0];
		var reader=new FileReader();
		reader.onload=function(evt){
			$.ajax('/upload',{
				method:'post',
				dataType:'text',
				data:{data:evt.target.result.replace(/data:.+?,/, "")}
			}).done(function(data){
				$scope.testsuite.push({
					comments:"",
					expected:[],
					id:(nextId++),
					url:data
				});
				$scope.$apply();
			});
		};
		reader.readAsDataURL(f);
	});
});

/*function DOMElement(testcase){
	var div=$('<div class="testcase"></div>');
	div.html($('#testcase-template').html());
	var comments=div.find('.comments');
	comments.val(testcase.comments);
	comments.change(function(){
		testcase.comments=comments.val();
	});
	comments.keydown(function(evt){
		if(evt.keyCode===13){//Enter key
			comments.blur();
		}
	});
	
	if(testcase.result){
		div.addClass(PassingTest(testcase)?'passing':'failing');
	}
	div.find('.display').css('background-image', "url("+testcase.url+")");
	div.find('.display > img').attr('src', testcase.url);
	div.click(function(evt){
		if(
			!div.hasClass('selected') && (
				$(evt.target).hasClass('display') ||
				$(evt.target).hasClass('testcase') ||
				$(evt.target).hasClass('display-overlay')
			)
		){
			div.addClass('selected');
		}
	});
	
	div.find('.back-button').click(function(evt){
		div.removeClass('selected');
		evt.preventDefault();
	});
	return div;
}*/

function RunTest(img){
	var deferred=Q.defer();
	$.post(
		'/run-test',
		{img:img}
	).done(function(data){
		if(data.indexOf('DONE')>=0){
			return deferred.resolve(null);
		}
		var arr=_.map(
			_.reject(
				data.split("\n"), function(x){return x==='';}
			), function(line){
				var parts=_.map(_.reject(
					_.invoke(line.split(' '), 'trim'),
					function(x){return x==="";}
				), parseFloat);
				return {
					center:{x: parts[0], y:parts[1]},
					radius:parts[2]
				};
			}
		);
		deferred.resolve(arr);
	});
	return deferred.promise;
}