var net=require('net'),
	child_process=require('child_process');

var PATH_TO_EXE=__dirname+'/run.sh';

var processed="";
var chld=null;

function stage_processing(){
	chld=child_process.spawn(PATH_TO_EXE, [], {
		cwd:__dirname,
		stdio: [null, 'pipe', process.stderr],
		detached: false
	});
	var out="";
	chld.stdout.on('data', function(data){
		out+=data;
		//This is something of a hack, in that it will not free memory
		var arr=out.split("\n");
		if(arr.length>=2){
			processed=arr[arr.length-2];//The last line won't be delimited yet
		}
	});
	chld.on('close', function(code){
		if(code!==0){
			console.log('error running test with code '+code);
		}
		chld=null;
		process.nextTick(stage_processing);
	});
}

process.on('SIGINT', function(){
	if(chld){
		chld.kill('SIGKILL');//Let's just make sure it happens
	}
	process.kill(0);
});

stage_processing();


var server=net.createServer(function(socket){
	socket.setEncoding('utf8');
	var times=0;
	function write(){
		if(times===0){
			return;
		}
		times--;
		socket.write(processed, "utf8", write);
	}
	socket.on('data', function(chunk){
		var oldTimes=times;
		for(var i=0;i<chunk.length;i++){
			if(chunk.charAt(i)==='\n'){
				++times;
			}
		}
		if(oldTimes===0){
			write();
		}
	});
});

server.listen(443, '0.0.0.0');
