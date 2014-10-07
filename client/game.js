'use strict';

function Player(nickName, spawned)
{
	this.spawned = spawned === null ? false : spawned;
	this.parts = [];
	this.nickName = nickName;
}

Player.prototype.getNickname = function()
{
	return this.nickName;
};

Player.prototype.addPart = function(a, b)
{
	this.parts.push({x:a, y:b});
};

Player.prototype.setPart = function(index, x, y)
{
	this.parts[index].x = x;
	this.parts[index].y = y;
};

Player.prototype.getParts = function()
{
	return this.parts;
};

Player.prototype.getPart = function(index)
{
	return this.parts[index];
};

Player.prototype.getPartCount = function()
{
	return this.parts.length;
};

Player.prototype.setHeadPart = function(x, y)
{
	this.setPart(0, x, y);
};

Player.prototype.getHeadPart = function()
{
	return this.getPart(0);
};

Player.prototype.isSpawned = function()
{
	return this.spawned;
};

Player.prototype.setSpawned = function(spawned)
{
	this.spawned = spawned;
};

var PacketHeader = {
    AUTH_REQUEST : 0,
    AUTH_RESPONSE : 1,
    CRITICAL_ERROR : 2,
    INITIAL_INFO_REQUEST : 3,
    INITIAL_INFO_RESPONSE : 4,
    TICK_INFO : 5,
    DIRECTION_CHANGE : 6,
    PLAYER_JOIN : 7,
	PLAYER_SPAWN : 8,
	PLAYER_LEAVE : 9
};

var connection;
var nickName;
var canvasCtx;
var textures = {};
var players = {};
var gameStarted = false;

function onOpen()
{
	
}

function onMessage(e)
{
	var recvData = new Uint8Array(e.data);

	switch(recvData[0])
	{
		case PacketHeader.TICK_INFO:
		{
			onTick(recvData.subarray(1));
			break;
		}
		case PacketHeader.PLAYER_SPAWN:
		{
			onPlayerSpawn(recvData.subarray(1));
			break;
		}
		case PacketHeader.PLAYER_JOIN:
		{
			onPlayerJoin(recvData.subarray(1));
			break;
		}
		case PacketHeader.PLAYER_LEAVE:
		{
			onPlayerLeave(recvData.subarray(1));

			break;
		}
		case PacketHeader.AUTH_REQUEST:
		{
			onAuthRequest();
			break;
		}
		case PacketHeader.INITIAL_INFO_REQUEST:
		{
			onInitialInfo(recvData.subarray(1));
			break;
		}
		case PacketHeader.CRITICAL_ERROR:
		{
			onError({message : String.fromCharCode.apply(null, recvData.subarray(1))});
			break;
		}
	}
}

function onTick(data)
{
	if(gameStarted)
	{
		var accumulator = 1;

		for(var i = 0; i < data[0]; ++i)
		{
			if(data[accumulator+1] & (1 << 0))
			{
				processTickForPlayer({player: players[data[accumulator]], newX: data[accumulator+2], newY: data[accumulator+3]});
				accumulator += 2;
			}

			accumulator += 2;
		}

		updateGraphics(canvasCtx);
	}
}

function onPlayerSpawn(data)
{
	var size = new Uint32Array(data.subarray(1, 5))[0];

	for(var i = 0; i < size; ++i)
		players[data[0]].addPart(data[5+2*i], data[5+2*i+1]);

	players[data[0]].setSpawned(true);
}

function onPlayerJoin(data)
{
	debug("Player joined: "+String.fromCharCode.apply(null, data.subarray(2, 2+data[1]))+"("+data[0]+")");

	players[data[0]] = new Player(String.fromCharCode.apply(null, data.subarray(2, 2+data[1])));
}

function onPlayerLeave(data)
{
	debug("Player left: "+players[data[0]].getNickname());

	delete players[data[0]];
}

function onAuthRequest(data)
{
	var nickSplit = stringToOctetArray(nickName);
	connection.send(new Uint8Array([PacketHeader.AUTH_RESPONSE, nickSplit.length].concat(nickSplit)));

	canvasCtx = initializeGraphics();
	drawBackground(canvasCtx);
}

function onInitialInfo(data)
{
	var accumulator = 1;

	for(var i = 0; i < data[0]; ++i)
	{
		var id = data[accumulator];

		players[id] = new Player(String.fromCharCode.apply(null, data.subarray(accumulator+2, accumulator+2+data[accumulator+1])), data[2+data[accumulator+1]]);

		accumulator += 2+data[accumulator+1]+1;
		var num = new Uint32Array(data.subarray(accumulator, accumulator+4))[0];
		accumulator += 4;

		var j;

		for(j = 0; j < num; j++)
			players[id].addPart(data[accumulator+j*2], data[accumulator+j*2+1]);

		accumulator += j*2;
	}

	loadTextures(canvasCtx);
}

function processTickForPlayer(data)
{
	if(data.newX == data.player.getHeadPart().x && data.newY == data.player.getHeadPart().y)
		return;

	canvasCtx.drawImage(textures.tile, 1+data.player.getPart(data.player.getPartCount()-1).x*8, 1+data.player.getPart(data.player.getPartCount()-1).y*8);

	for(var i = data.player.getPartCount()-1; i !== 0; --i)
		data.player.setPart(i, data.player.getPart(i-1).x, data.player.getPart(i-1).y);

	data.player.setHeadPart(data.newX, data.newY);
}

function onClose()
{
	debug("server closed connection");
	gameStarted = false;
}

function onError(e)
{
	alert("Error: "+e.message);
}

function stringToOctetArray(str)
{
	var array = [];

	for(var i=0;i<str.length;++i)
		array.push(str.charCodeAt(i));

	return array;
}

function processButtonClick(button)
{
	if(button.id != 'play' || document.getElementById('nick').value.length < 3)
		return;

	nickName = document.getElementById('nick').value;

	connection = new WebSocket('ws://37.187.39.141:7777', ['snkmp']);
	connection.binaryType = 'arraybuffer';
	connection.onopen = onOpen;
	connection.onclose = onClose;
	connection.onmessage = onMessage;
}

function initializeGraphics()
{
	document.getElementById('main').innerHTML = '<canvas id="canvas" width="513" height="513"/>';
	return document.getElementById('canvas').getContext('2d');
}

window.onkeydown = function(e)
{
	if(gameStarted)
	{
		switch(e.keyCode)
		{
			case 38: //up
			{
				connection.send(new Uint8Array([PacketHeader.DIRECTION_CHANGE, 0]));
				break;
			}
			case 40: //down
			{
				connection.send(new Uint8Array([PacketHeader.DIRECTION_CHANGE, 1]));
				break;
			}
			case 37: //left
			{
				connection.send(new Uint8Array([PacketHeader.DIRECTION_CHANGE, 2]));
				break;
			}
			case 39: //right
			{
				connection.send(new Uint8Array([PacketHeader.DIRECTION_CHANGE, 3]));
				break;
			}
		}
	}
};

function drawBackground(ctx)
{
	ctx.fillStyle = 'rgb(196, 195, 210)';
	ctx.fillRect(0, 0, document.getElementById('canvas').width, document.getElementById('canvas').height);
}

function loadTextures(ctx)
{
	textures.tile = new Image(7,7);
	textures.tile.src = 'tile.png';
	textures.tile.onload = function()
	{
		drawTiles(ctx);
		updateGraphics(ctx);

		var sendData = new Uint8Array([PacketHeader.INITIAL_INFO_RESPONSE]);
		connection.send(sendData);

		gameStarted = true;
	};
}

function drawTiles(ctx)
{
	for(var i=0;i<64;++i)
		for(var j=0;j<64;++j)
			ctx.drawImage(textures.tile, 1+j*8, 1+i*8);

}

function updateGraphics(ctx)
{
	for(var id in players)
	{
		if(players[id].isSpawned())
		{
			ctx.fillStyle = "rgb(77, 77, 109)";
			for(var pId in players[id].getParts())
			{
				if(players[id].getParts().hasOwnProperty(pId))
				{
					ctx.fillRect(1+8*players[id].getPart(pId).x, 1+8*players[id].getPart(pId).y, 7, 7);
				}
			}
		}
	}
}

function debug(message)
{
	document.getElementById('debug').innerHTML = message;
}
