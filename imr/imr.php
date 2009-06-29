<?
// Austins PHP IMR v2
// DEBUG ERRORS WITH : mysql_query("INSERT INTO `debug` (`string`) VALUES ('$_SERVER[QUERY_STRING]')",$db);

// MYSQL CONFIG FILE : dbconnect.php
include "dbconnect.php";

// CONNECT TO THE MYSQL
$db = mysql_connect($serverip, $serverusername, $serverpassword);
mysql_select_db($databasename,$db);

// DECLARATIONS
$MAX_PLAYERS 		= 50; 							// MOST AMOUNT OF PLAYERS ALLOWED IN THE IMR
$MAX_PER_GAME 		= 10; 							// MOST AMOUNT OF PLAYERS ALLOWED IN THE GAME
$MAX_GAMES 			= 15; 							// MOST AMOUNT OF GAMES ALLOWED IN THE IMR
$TIMEOUT   			= time() - 15;   				// HOW MANY SECONDS BEFORE A PLAYER HAS TIMED OUT
$OLD_USER_REMOVAL 	= time() - 60*60*24;			// HOW MANY SECONDS TO REMOVE OLD USERS FROM THE OLDUSERLIST TABLE (60*60*24=A DAY)
$URL 				= "http://www.hoverrace.com";	// WEBSITE ADDRESS

// DISECTING THE QUERY STRING FOR FUTURE USAGE
$CMD = split("%%", $_SERVER["QUERY_STRING"]);

// IF NO QUERY STRING, THEN STOP WITH ERROR 201: Not on-line
if(!$_SERVER["QUERY_STRING"]) ErrorCode(201);

// STRIP THE =
$CMD[0] = substr($CMD[0], 1, strlen($CMD[0])-1);

// IF THE CLIENT IS NOT HOVERRACE AND THEY ARE NOT ACCESSING THE USERLIST, TURN THEM AWAY
if(GetClient() != "HoverRace" AND $CMD[0] != "WWW_ULIST") exit;

// PERFORM THE FUNCTION
$CMD[0]($db, $CMD);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REFRESHING THE IMR                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function REFRESH($db, $CMD) {

	// UPDATE AND PRINT USERLIST
	// STRIP - OFF OF THE USERID
    $refreshid = split("-",$CMD[1]);

    // CHECK IF THEY STILL EXIST ON THE USERLIST OR KICK THEM OUT OF THE ROOM
    $result = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$refreshid[0]'",$db);
	if ($myrow = mysql_fetch_array($result))
	{
	    $PLU = $myrow["PlayerListUpdate"];
	    $GLU = $myrow["GameListUpdate"];
	    $MTS = $myrow["MessageTimeStamp"];
	} else ErrorCode(201);

	// CHECK IF THEY TIMED OUT AND REMOVE THEM FROM THE DATABASE
	$result = mysql_query("SELECT * FROM `userlist` WHERE `TimeStamp` < '$GLOBALS[TIMEOUT]'",$db);
	if ($myrow = mysql_fetch_array($result)) {
		do
		{
			ChatMessage(0,ChatDate(),"$myrow[UserName] has timed out!");
			mysql_query("DELETE FROM `userlist` WHERE `UserID` = '$myrow[UserID]' LIMIT 1",$db);
			mysql_query("DELETE FROM `gameplayers` WHERE `UserID` = '$myrow[UserID]' LIMIT 1",$db);
			mysql_query("DELETE FROM `gamelist` WHERE `UserID` = '$myrow[UserID]' LIMIT 1",$db);
			// REMOVE THEM FROM HOSTING A GAME
			mysql_query("DELETE FROM `gameleave` WHERE `HostName` = '$myrow[UserName]'",$db);
		} while ($myrow = mysql_fetch_array($result));
		// FORCES A RELOAD ON THE USERLIST AND GAMELIST
		mysql_query("UPDATE `userlist` SET `PlayerListUpdate` = '0', `GameListUpdate` = '0'",$db);
		// FORCE THE PLAYER TO LOAD THEM NOW (OTHERWISE HIS PLU AND GLU WOULD BE RETURNED TO 1 WITHOUT RELOADING THE LISTS!)
		$PLU = 0;
		$GLU = 0;
	}

	// REMOVE USERS IN THE OLDUSERLIST TABLE
	mysql_query("DELETE FROM `olduserlist` WHERE `TimeStamp` < '$GLOBALS[OLD_USER_REMOVAL]'",$db);

	// UPDATE THE PLAYERS LIST IF NECESSARY
	if ($PLU == 0) PlayerListUpdate();

	// UPDATES THE GAME LIST IF NECESSARY, GameListUpdate will return TRUE if the players game has been checked for connectivity
	// to delay the update on the game list
	if ($GLU == 0) if (!GameListUpdate($refreshid[0])) $GLU = 1;
	
	// CHECK IF THERE ARE ANY NEW MESSAGES TO DISPLAY
	$result = mysql_query("SELECT * FROM `chat` WHERE `MessageID` > '$MTS' ORDER BY `MessageID`",$db);
	// PRINT NEW MESSAGES
	if ($myrow = mysql_fetch_array($result)) {
        do {
            $show = TRUE;
            
        	// PRIVATE MESSAGE FILTERS
        	if ($myrow["ToUser"] AND $refreshid[0]==$myrow["ToUser"]) $priv = " <private>";
        	elseif ($myrow["ToUser"] AND $refreshid[0] != $myrow["ToUser"]) $show = FALSE;
        	
        	if ($show)
        	{
        	    // PRINTS OFF THE NEW MESSAGE AND WHO IT IS BY
             	PrintIMR("CHAT\n" . urldecode($myrow["UserName"]) . "$priv" . GetChatCode() . " ");
             	$message = $myrow["Message"];
				$message = str_replace("+","%2B",$message);
				//$message = str_replace("\'","'",$message);
				PrintIMR(urldecode($message). "\n");
        	}
        	
        	$mid = $myrow["MessageID"];
        } while ($myrow = mysql_fetch_array($result));
        $midq = ", `MessageTimeStamp` = '$mid'";
	}
	
	// UPDATE THE PLAYERS TIMESTAMP SHOWING HE IS ACTIVE IN THE IMR, ALSO SETS THEIR UPDATES TO 1, AND IF ANY NEW
	// MESSAGES WERE SENT, IT UPDATES THEIR MESSAGE TIMESTAMP TO REFLECT
	mysql_query("UPDATE `userlist` SET `TimeStamp` = '" . time() . "', `PlayerListUpdate` = '1', `GameListUpdate` = '$GLU'$midq WHERE `UserID` = '$refreshid[0]'",$db);

 	// AFTER WHATEVER THE CHAT REMOVAL IS SET TO IN SECONDS, THE CHAT MESSAGES ARE DELETED FROM THE DATABASE
 	// TO SAVE IT BECOMING HUGE
  	// mysql_query("DELETE FROM `chat` WHERE `TimeStamp` < '$chatrem' ORDER BY 'TimeStamp' DESC OFFSET 10",$db);

  	PrintIMR("SUCCESS\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADDING THE USER 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function ADD_USER($db, $CMD) {

	$PlayerIP = GetPlayerIP();
	
    // CHECK FOR OLD CLIENTS AND REMOVE THEM, ALSO STOPS IMR FILLING
   	mysql_query("DELETE FROM `userlist` WHERE `TimeStamp` < '$GLOBALS[TIMEOUT]'",$db);
   	// CHECK FOR SAME NAME AND ADD A NUMBER ONTO THE END IF THERE IS A SAME NAME IN THE IMR
   	$result = mysql_query("SELECT * FROM `userlist` WHERE `UserName` = '$CMD[5]'",$db);
   	if ($myrow = mysql_fetch_array($result)) {
  		do {
  		    $count++;
  		    $alteredcommand5 = $CMD[5] . $count;
            $resultcheck = mysql_query("SELECT * FROM `userlist` WHERE `UserName` = '$alteredcommand5'",$db);
            if (($myrowcheck = mysql_fetch_array($resultcheck))==FALSE) {
                $UniqueName=TRUE;
            }
		} while ($UniqueName==FALSE);
		$CMD[5]=$alteredcommand5;
   	}
   	
	// CHECK HOW MANY PEOPLE ARE IN THE IMR AND ASSIGN A UNIQUE USERID
	$result = mysql_query("SELECT * FROM `userlist` ORDER BY `UserID`",$db);
    if ($myrow = mysql_fetch_array($result)) {
        do {
        	if($myrow["UserID"] - $UserID > 1) break;
        	else $UserID = $myrow["UserID"];
        } while ($myrow = mysql_fetch_array($result));
    }
    $UserID++;
    
	// GIVE AN ERROR IF THERE ARE ALREADY THE MAXIMUM AMOUNT OF CLIENTS IN THE ROOM
	if ($UserID > $GLOBALS["MAX_PLAYERS"])	ErrorCode(102);

	// TELL THE CLIENT THAT JOINING WAS A SUCCESS
	PrintIMR("SUCCESS\n");
	PrintIMR("USER_ID $UserID\n");
	// RETRIEVE THE TIMESTAMP OF THE LAST MESSAGE ON THE IMR, SO THAT THE REFRESH COMMAND PICKS UP NEW MESSAGES
	$resultmts = mysql_query("SELECT * FROM `chat` ORDER BY `MessageID` DESC LIMIT 1",$db);
    if ($myrowmts = mysql_fetch_array($resultmts))  $MessageTimeStamp=$myrowmts["MessageID"];
	// INSERT USER INTO DATABASE CHECKING FOR HIS OLD IP ADDRESS SETTING IN OUR OLDUSERLIST TABLE
	$resultip = mysql_query("SELECT * FROM `olduserlist` WHERE `UserName` = '$CMD[5]'",$db);
    if ($myrowip = mysql_fetch_array($resultip)) {
        // TELL THE IMR A NEW PLAYER HAS JOINED
        $AltIP = $myrowip["IPAddress"];
        mysql_query("DELETE FROM `olduserlist` WHERE `UserName`='$CMD[5]'",$db);
    } else $AltIP = $PlayerIP;
    
    ChatMessage(0,ChatDate(),"$CMD[5] has joined the IMR! ($AltIP) [" . GetUserAgent() . "]");
    mysql_query("INSERT INTO `userlist` (`UserName`,`UserID`,`RegKey`,`Version`,`Key2`,`Key3`,`IPAddress`,`IPAddressOriginal`,`TimeStamp`,`MessageTimeStamp`,`JoinedIMR`,`PlayerListUpdate`,`GameListUpdate`) VALUES ('$CMD[5]','$UserID','$CMD[1]','$CMD[2]','$CMD[3]','$CMD[4]','$AltIP','$PlayerIP','". time() ."','$MessageTimeStamp','". time() ."','1','1')",$db);

	// TELLS EVERYONE ELSE TO UPDATE
	mysql_query("UPDATE `userlist` SET `PlayerListUpdate` = '0'",$db);

	// CHECKS IF ANY OF THE CLIENT SLOTS ARE TAKEN
	PlayerListUpdate();

    // PRINT GAMES TO THE GAMES LIST
	GameListUpdate($UserID);

	// ADDS TO THE SMALL INTRO MESSAGE
    PrintIMR("CHAT\n--\n");

    // PRINT THE INTRO MESSAGE IF ONE EXISTS
    $resultint = mysql_query("SELECT * FROM `intro`",$db);
    if ($myrowint = mysql_fetch_array($resultint)) {
        $message = $myrowint["Message"];
        $message = str_replace("\n", "\nCHAT\n", $message);
        PrintIMR("CHAT\n$message\n");
        PrintIMR("CHAT\n--\n");
    }


}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REMOVING THE USER FROM THE IMR 																			 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function DEL_USER($db, $CMD) {

	// REMOVE THE CLIENT FROM THE IMR AS HE LEAVES
	$leaveid = split("-",$CMD[1]);
	$resultmts = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$leaveid[0]'",$db);
	$myrowmts = mysql_fetch_array($resultmts);
	// ADD THEM TO A STORED DATABASE SO THAT THEIR IP ADDRESS IS REMEMBERED IF THEY HAVE CHANGED IT
	if ($myrowmts["IPAddress"]<>$myrowmts["IPAddressOriginal"]) {
		mysql_query("INSERT INTO `olduserlist` (`UserName`,`IPAddress`,`TimeStamp`) VALUES ('$myrowmts[UserName]','$myrowmts[IPAddress]','". time() ."')",$db);
	}
	
	// IF HE LEFT TO RACE, PUT HIM INTO A TABLE CONTAINING OTHER PLAYERS WHO JUST LEFT
	$resulthost = mysql_query("SELECT * FROM `gamelist` WHERE `UserID`='$leaveid[0]'",$db);
	$resultjoin = mysql_query("SELECT * FROM `gameplayers` WHERE `UserID`='$leaveid[0]'",$db);
	if ($myrowhost = mysql_fetch_array($resulthost))
	{
	    // PRINT THE MESSAGE SAYING PEOPLE ARE LEAVING TO PLAY
	    $resultleave = mysql_query("SELECT * FROM `gameleave` WHERE `GameID`='$myrowhost[GameID]'",$db);
	    $myrowleave = mysql_fetch_array($resultleave);
	    ChatMessage(0,ChatDate(),urldecode($myrowleave["HostName"]) . " has launched a game.");
		if ($myrowleave["Weapons"]==1) { $weapons = " with weapons."; } else { $weapons = " and no weapons."; }
		mysql_query("INSERT INTO chat (`UserID`,`UserName`,`Message`,`TimeStamp`) VALUES ('0','Game Details','$myrowleave[GameName], $myrowleave[Laps] laps$weapons','". time() ."')",$db);
	    // NOW PRINT THE PARTICIPANTS
	    $players = "<#1>" . urldecode($myrowleave["HostName"]);
	    // NOW ADD THE OTHERS IN ORDER
		$extraplayers = " " . $myrowleave["PlayerNames"];
		if ($extraplayers != " ")
		{
			for ($playersingame=2; $playersingame<=$GLOBALS["MAX_PER_GAME"]; $playersingame++)
			{
			    if (stripos($extraplayers, "[" . $playersingame . "]"))
			    {
			    	$splitplayer = split("\[$playersingame\]", $extraplayers);
			    	$splitplayer = split("\<$playersingame\>", $splitplayer[1]);
			    	$splitplayer = $splitplayer[0];
			    	$players .= ", <#$playersingame>" . urldecode($splitplayer);
				}
			}
		}
	    ChatMessage(0,"Game Players","$players");
	    mysql_query("DELETE FROM `gameleave` WHERE `GameID` = '$myrowhost[GameID]'",$db);
	} elseif (($myrowjoin = mysql_fetch_array($resultjoin))==FALSE) {
		// TELL THE ROOM HE HAS LEFT
		ChatMessage(0,ChatDate(),"$myrowmts[UserName] has left the IMR!");
	}
	
	// REMOVE HIM FROM THE USERLIST
  	mysql_query("DELETE FROM `userlist` WHERE `UserID` = '$leaveid[0]' LIMIT 1",$db);
  	// REMOVE HIM FROM ANY GAMES (BECAUSE WHEN YOU START A GAME, THEY ONLY LEAVE THE ROOM)
  	mysql_query("DELETE FROM `gamelist` WHERE `UserID` = '$leaveid[0]'",$db);
    mysql_query("DELETE FROM `gameplayers` WHERE `UserID` = '$leaveid[0]'",$db);
    // FORCES A RELOAD ON THE USERLIST AND GAMELIST (BECAUSE WHEN YOU START A GAME, THEY ONLY LEAVE THE ROOM)
    mysql_query("UPDATE `userlist` SET `PlayerListUpdate` = '0', `GameListUpdate` = '0'",$db);
	PrintIMR("SUCCESS\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADDING CHAT TO THE CHAT DATABASE 																		 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function ADD_CHAT($db, $CMD) {

    // STRIP - OFF OF THE USERID AND UPDATE THE TIMESTAMP
    $addchatid = split("-",$CMD[1]);
    $result = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$addchatid[0]'",$db);
    if ($myrow = mysql_fetch_array($result)) {
        $UserName = $myrow["UserName"];
    }

    if (substr(urldecode($CMD[2]),0,3)=="/ip") {
    	// LET A PLAYER CHANGE HIS IP (USEFUL IF YOU WANT TO USE HAMACHI)
    	/////////////////////////////////////////////////////////////////
    	$changedip = split ("/ip ",urldecode($CMD[2]));
		$resultmts = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$addchatid[0]'",$db);
		$myrowmts = mysql_fetch_array($resultmts);
		// IF NO IP IS GIVEN, SIMPLY CHANGE TO THEIR ORIGINAL IP
		if ($changedip[1]==FALSE) {
	    	$changedip[1] = GetPlayerIP();
		}
		if (is_numeric(str_replace(".", "", $changedip[1]))) {
		    mysql_query("UPDATE `userlist` SET `IPAddress` = '$changedip[1]' WHERE `UserID` = '$addchatid[0]'",$db);
		    mysql_query("UPDATE `gamelist` SET `GameIP` = '$changedip[1]' WHERE `UserID` = '$addchatid[0]'",$db);
    		ChatMessage(0,ChatDate(),"$myrowmts[UserName] has changed IP to: $changedip[1]");
    		
    		mysql_query("UPDATE `userlist` SET `GameListUpdate` = '0'",$db);
		} else {
		    ChatMessage(0,ChatDate(),"You cannot change your IP to text.",$addchatid[0]);
		}
	} elseif (substr(urldecode($CMD[2]),0,4)=="/msg") {
		// SENDING PRIVATE MESSAGES
		///////////////////////////

		// SPLIT THE USER/MESSAGE BIT AWAY
    	$split = split ("msg ",urldecode($CMD[2]),2);
    	// SPLIT THE USER AND MESSAGE INTO AN ARRAY
    	$contentsmessage = split (" ","$split[1]", 2);
    	// IF THE USER ID IS NOT ENTERED, THEN TELL THEM HOW TO MESSAGE
    	if(!is_numeric($contentsmessage[0]))
    	{
	    	ChatMessage(0,ChatDate() . "IMR","Usage: /msg <userid> <message>",$addchatid[0]);
	    	exit;
    	}
    	
		if($contentsmessage[0] == $addchatid[0])
		{
		    ChatMessage(0,ChatDate() . "IMR","Messaging yourself, eh?",$addchatid[0]);
		    exit;
		}
    	
    	// CHECK IF THE USER ID EXISTS
   		$resultmtsb = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$contentsmessage[0]'",$db);
		if ($myrowmtsb = mysql_fetch_array($resultmtsb)) {
    		// DELIVER THE MESSAGE
    		$message = $contentsmessage[1];
			ChatMessage($addchatid[0],ChatDate() . "$UserName",addslashes($message),$myrowmtsb["UserID"],$myrowmtsb["UserName"]);
			ChatMessage($addchatid[0],ChatDate() . "$UserName",addslashes($message),$addchatid[0],$myrowmtsb[UserName]);
		} else {
    		// TELL THEM THE PLAYER DOESN'T EXIST
    		ChatMessage(0,ChatDate() . "IMR","User ID $contentsmessage[0] does not exist.",$addchatid[0]);
		}
	} elseif (substr(urldecode($CMD[2]),0,4)=="/rec") {
		// VIEWING RECORDS FOR HOSTED TRACK
		///////////////////////////////////
		include "command-rec.php";
	// CODE FOR A STATUS CHANGE
	} elseif (substr(urldecode($CMD[2]),0,7)=="/status") {
		// LET A PLAYER CHANGE HIS STATUS
    	/////////////////////////////////
    	$changedstatus = split ("status ",urldecode($CMD[2]));
		$resultname = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$addchatid[0]'",$db);
		$myrowname = mysql_fetch_array($resultname);
		if ($changedstatus[1]) {
			ChatMessage(0,ChatDate(),"$myrowname[UserName] has changed their status to \'$changedstatus[1]\'");
		} elseif ($myrowname["Status"]) {
		    ChatMessage(0,ChatDate(),"$myrowname[UserName] has cleared their status");
		}
		mysql_query("UPDATE `userlist` SET `Status` = '$changedstatus[1]' WHERE `UserID` = '$addchatid[0]'",$db);
		// FORCES A RELOAD ON THE USERLIST AND GAMELIST (BECAUSE WHEN YOU START A GAME, THEY ONLY LEAVE THE ROOM)
    	mysql_query("UPDATE `userlist` SET `PlayerListUpdate` = '0', `GameListUpdate` = '0'",$db);
    } elseif (substr(urldecode($CMD[2]),0,3)=="/me") {
		// WRITE A /ME STYLE MESSAGE
		////////////////////////////
		$mecommand = split ("me ",urldecode($CMD[2]),2);
		$resultname = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$addchatid[0]'",$db);
		$myrowname = mysql_fetch_array($resultname);
		if ($mecommand[1]) {
			ChatMessage(0,ChatDate(),"$myrowname[UserName] $mecommand[1]");
		}
	} elseif (substr(urldecode($CMD[2]),0,6)=="/clear") {
		// CLEAR THE SCREEN
		///////////////////
		$ic=0;
		ChatMessage(0,"","\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\nCHAT\n\n",$addchatid[0]);
		ChatMessage(0,"","*Screen cleared*",$addchatid[0]);
	} elseif (substr(urldecode($CMD[2]),0,16)=="/phra-totalranks") {
		// VIEW TOP 5 PHRA TOTAL RANKINGS
		/////////////////////////////////
		//include "command-phratotalranks.php";

	} elseif (substr(urldecode($CMD[2]),0,1)=="/") {
        // TELL THEM THIS COMMAND DOESN'T EXIST
        ///////////////////////////////////////
	    ChatMessage(0,ChatDate() . "IMR","This command does not exist.",$addchatid[0]);
	} elseif ($CMD[2]) {
 		// INSERT THE CHAT TO THE IMR
 		/////////////////////////////
    	ChatMessage($CMD[1],ChatDate() . "$UserName",addslashes($CMD[2]));
	}
	PrintIMR("SUCCESS\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADDING A GAME TO THE DATABASE 																			 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function ADD_GAME($db, $CMD) {

	// FIND A SUITABLE SLOT OUT OF THE AVAILABLE GAME SPACES AVAILABLE TO ADD THE GAME INTO
    $result = mysql_query("SELECT * FROM `gamelist` ORDER BY `GameID`",$db);
    if ($myrow = mysql_fetch_array($result))
    {
        do {
            if ($myrow["GameID"] - $GameID > 1) break;
            else $GameID = $myrow["GameID"];
        } while ($myrow = mysql_fetch_array($result));
    }
    $GameID++;
    
	if ($GameID <= $GLOBALS["MAX_GAMES"]) {
	    // IF THERE IS NO GAME IN THE $GAMECOUNT SLOT, IT WILL PUT THE CLIENTS GAME HERE
	    // STRIP - OFF OF THE USERID AND UPDATE THE TIMESTAMP
		$addgameid = split("-",$CMD[1]);
        $resultmts = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$addgameid[0]'",$db);
		$myrowmts = mysql_fetch_array($resultmts);
		$ip = $myrowmts["IPAddress"];

	    mysql_query("INSERT INTO `gamelist` (`GameID`,`UserID`,`GameName`,`Track`,`Laps`,`Weapons`,`GameIP`,`Port`,`Version`) VALUES ('$GameID','$CMD[1]','" . addslashes($CMD[2]) ."','" . addslashes($CMD[3]) ."','$CMD[4]','$CMD[5]','$ip','$CMD[6]', '" . GetUserAgent() . "')",$db);
	    mysql_query("INSERT INTO `gameleave` (`GameID`,`HostName`,`GameName`,`Laps`,`Weapons`) VALUES ('$GameID','$myrowmts[UserName]','" . addslashes($CMD[2]) ."','$CMD[4]','$CMD[5]')",$db);
	    mysql_query("INSERT INTO `gameplayers` (`GameID`,`UserID`,`JoinOrder`) VALUES ('$GameID','$CMD[1]','1')",$db);
	    // THIS TELLS THE GAMELIST TO UPDATE ON REFRESH
	    mysql_query("UPDATE `userlist` SET `GameListUpdate` = '0'",$db);
		PrintIMR("SUCCESS\n");
		PrintIMR("GAME_ID $GameID-$CMD[1]\n");
	} elseif ($GameID > $GLOBALS["MAX_GAMES"]) {
	    // THIS WARNS THAT THERE ARE ALREADY ALL GAME SLOTS FILLED
	    ErrorCode(402);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHECK A GAMES CONNECTIVITY TO ENSURE IT WORKS															 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function CHECK_GAME($db, $CMD) {

    if($CMD[4] != -1)
	{
		$resultmts = mysql_query("SELECT * FROM `gamelist` WHERE `GameID` = '$CMD[4]' AND `UserID` = '$CMD[1]'",$db);
		$myrowmts = mysql_fetch_array($resultmts);
		$gamename = $myrowmts["GameName"];
	}
	if(@fsockopen($CMD[2], $CMD[3], $errno, $errstr, '4'))
	{
		if($CMD[4] != -1) $Delim = "+";
	}
	else
	{
	    $resultmts = mysql_query("SELECT * FROM `userlist` WHERE `UserID` = '$CMD[1]'",$db);
		if($myrowmts = mysql_fetch_array($resultmts))
		{
		    if($CMD[4] != -1) ChatMessage($CMD[1],ChatDate() . "IMR","$myrowmts[UserName]\'s game might have connection issues!");
		    ChatMessage($CMD[1],ChatDate() . "IMR", "You appear to be behind a router or firewall:  Visit: http://www.hoverrace.com/?page=unabletoconnect",$CMD[1]);
		    $Delim = "-";
		}
	}
	if($CMD[4] != -1) mysql_query("UPDATE `gamelist` SET `GameName` = '" . urlencode("$Delim") . " $gamename' WHERE `GameID` = '$CMD[4]' AND `UserID` = '$CMD[1]'",$db);
	
	mysql_query("UPDATE `userlist` SET `PlayerListUpdate` = '0', `GameListUpdate` = '0'",$db);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REMOVING A GAME FROM THE DATABASE 																		 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function DEL_GAME($db, $CMD) {

	// DELETES THE GAME FROM THE GAMELIST
    mysql_query("DELETE FROM `gamelist` WHERE `GameID` = '$CMD[1]' AND `UserID` = '$CMD[2]' LIMIT 1",$db);
    mysql_query("DELETE FROM `gameleave` WHERE `GameID` = '$CMD[1]' LIMIT 1",$db);
    mysql_query("DELETE FROM `gameplayers` WHERE `GameID` = '$CMD[1]'",$db);
    // TELLS THE CLIENT TO UPDATE HIS GAMELIST
    mysql_query("UPDATE `userlist` SET `GameListUpdate` = '0'",$db);
    PrintIMR("SUCCESS\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JOINING A GAME 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function JOIN_GAME($db, $CMD) {

	// CHECK IF THE HOST HAS THE SAME VERSION AS THE CLIENT
 	$resultmts = mysql_query("SELECT * FROM `gamelist` WHERE `GameID` = '$CMD[1]' AND `Version` = '" . GetUserAgent() . "'",$db);
 	if (($myrow = mysql_fetch_array($resultmts))) {
		// ASSIGN A PLAYER A SLOT IN THE GAME FROM 2-10 (JOINORDER IS SIMPLY FOR DISPLAY PURPOSES ON THE PLAYERLIST)
		$oldpid = 1;
		
		$resultmts = mysql_query("SELECT * FROM `gameplayers` WHERE `GameID` = '$CMD[1]' ORDER BY `JoinOrder`",$db);
		if ($myrow = mysql_fetch_array($resultmts)) {
			do {
			    if ($myrow["JoinOrder"] - $oldpid > 1) break;
			    else $oldpid = $myrow["JoinOrder"];
			} while (mysql_fetch_array($resultmts));
		}
		$oldpid++;
		
		if($oldpid <= $GLOBALS["MAX_PER_GAME"])
		{
			// INSERT INTO THE GAMEPLAYERS DATABASE (FOR DISPLAYING ON THE PLAYERLIST)
    		mysql_query("INSERT INTO `gameplayers` (`GameID`,`UserID`,`JoinOrder`) VALUES ('$CMD[1]','$CMD[2]','$oldpid')",$db);
    		// REFRESH THE GAMES PLAYERS FOR WHEN THEY LEAVE THE IMR
    		RefreshPlayerNames($CMD[1]);
    		// TELLS THE CLIENT TO UPDATE HIS GAMELIST
    		mysql_query("UPDATE `userlist` SET `GameListUpdate` = '0'",$db);
    		// CHECK THE PLAYER CAN CONNECT
    		if($CMD[3] and $CMD[4]) file_get_contents("http://" . $_SERVER["SERVER_ADDR"] . $_SERVER['PHP_SELF'] . "?=CHECK_GAME%%$CMD[2]%%$PlayerIP%%$CMD[3]%%-1");
		} else {
			ErrorCode(503);
		}
		
		PrintIMR("SUCCESS\n");
 	} else ErrorCode(103);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LEAVING A GAME 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function LEAVE_GAME($db, $CMD) {

	// REMOVE THEMSELVES FROM THE GAMEPLAYERS DATABASE(AND THE GAMES PLAYERLIST)
    mysql_query("DELETE FROM `gameplayers` WHERE `GameID` = '$CMD[1]' AND `UserID` = '$CMD[2]'",$db);
    // TELLS THE CLIENT TO UPDATE HIS GAMELIST
    mysql_query("UPDATE `userlist` SET `GameListUpdate` = '0'",$db);
    
	RefreshPlayerNames($CMD[1]);
        
    PrintIMR("SUCCESS\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DISPLAYING THE PEOPLE IN THE IMR 																							 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

function WWW_ULIST($db, $CMD) {

	// CALLS THE USERLIST
    $result = mysql_query("SELECT * FROM `userlist` ORDER BY `UserID`",$db);
    $myrow = mysql_fetch_array($result);
    // FAKE THE OLD LOOK OF THE USERLIST FOR THE SAKE OF THE IMR DISPLAY ON HR.COM
    print "<HTML>";
	print "<meta http-equiv=\"Refresh\" Content=8 >\n";
	print "<BODY BGCOLOR=\"FFFFFF\" TEXT=\"000000\">\n";
	print "<table border=\"2\" bgcolor=\"E0DFE3\"><tr><td align=\"center\" width=\"150\">Users List</td></tr></table>\n";
	print "<pre><FONT FACE=\"MS Sans Serif\" STYLE=\"font-size : 8 pt\">\n";
	// PRINT EACH NAME
	do {
	print urldecode($myrow[UserName]) . "\n";
	} while ($myrow = mysql_fetch_array($result));
	// END OF FAKING
	print "</font></pre>\n";
	print "</BODY></HTML>";
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

function PrintIMR($str)
{
	print $str;
}

function GetClient()
{
	if(GetUserAgent() == "1.23") return "HoverRace";
	else {
	    $u = $_SERVER['HTTP_USER_AGENT'];
		$u = split("/", $u);
		return $u[0];
	}
}

function GetUserAgent()
{
	$u = $_SERVER['HTTP_USER_AGENT'];

	if(!$u)	return "1.23";
	else {
		$u = split(" ", $u);
		$u = split("/", $u[0]);
		return $u[1];
	}
}

function GetPlatform()
{
	if(GetUserAgent() == "1.23") return "Win32";
	else {
	    $u = $_SERVER['HTTP_USER_AGENT'];
		$u = split(" ", $u);
		return $u[1];
	}
}

function GetChatCode()
{
	if(GetUserAgent() == "1.23") return "»";
    else return utf8_encode("»");
}

function GetPlayerIP()
{
    return isset($_SERVER['HTTP_X_FORWARDED_FOR']) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : $_SERVER['REMOTE_ADDR'];
}

//////////////////////////////////////////////////////////////////////////////////////
// UPDATE THE GAMELEAVE TABLE'S "PLAYERNAMES" COLUMN TO REFLECT YOUR LEAVING
//////////////////////////////////////////////////////////////////////////////////////
function RefreshPlayerNames($gameid)
{
    // LOOP THE PLAYERS STILL IN THE GAME
    $newpn = "";
    $resultgameleaveadd = mysql_query("SELECT * FROM `gameplayers` WHERE `GameID` = '$gameid' AND `JoinOrder` > '1'",$GLOBALS["db"]);
    if ($myrowleave = mysql_fetch_array($resultgameleaveadd))
    {
        do
		{
		    // GET THE PLAYERS NAME AND ADD THEM TO THE ARRAY
		    $resultgameleavename = mysql_query("SELECT * FROM `userlist` WHERE `UserID`='$myrowleave[UserID]'",$GLOBALS["db"]);
    		$myrowname = mysql_fetch_array($resultgameleavename);
    		$newpn = $newpn . "[$myrowleave[JoinOrder]]$myrowname[UserName]<$myrowleave[JoinOrder]>";
		} while ($myrowleave = mysql_fetch_array($resultgameleaveadd));
    }
    // UPDATE "PLAYERNAMES"
    mysql_query("UPDATE `gameleave` SET `PlayerNames` = '$newpn' WHERE `GameID` = '$gameid'",$GLOBALS["db"]);
}

function ChatDate()
{
	return "<" . date("H:i:s") . "> ";
}

function ErrorCode($errno)
{
	print "ERROR $errno\n";
	exit;
}

function GameListUpdate($UserID)
{
	$db = $GLOBALS["db"];
    // PRINT GAMES TO THE GAMES LIST
    $result = mysql_query("SELECT * FROM `gamelist` ORDER BY `GameID`",$db);
	if($myrow = mysql_fetch_array($result)) {
		do {
	        if ($myrow["GameID"]-$oldgid > 1)
	        {
	            for ( $x = $oldgid+1; $x < $myrow["GameID"]; $x++ ) PrintIMR("GAME $x DEL\n");
	        }
		    // COUNTS THE PLAYERS IN THE GAME, AND MAKES THE PRINT FOR LATER
		    $playercount=0;
		    $playerlist=FALSE;
		    $resultshowpl = mysql_query("SELECT * FROM `gameplayers` WHERE `GameID` = '$myrow[GameID]' ORDER BY `JoinOrder`",$db);
	    	if ($myrowshowpl = mysql_fetch_array($resultshowpl)) {
	    	    do {
	    			$playerlist .= "$myrowshowpl[UserID] ";
	    			$playercount++;
	    	    } while ($myrowshowpl = mysql_fetch_array($resultshowpl));
	    	}
		    // TELLS THE IMR TO ADD THIS GAME TO THE GAME LIST
	    	PrintIMR("GAME $myrow[GameID] NEW $myrow[UserID]\n");
	    	PrintIMR(stripslashes(urldecode($myrow["GameName"])) . "\n");
	    	PrintIMR(stripslashes(urldecode($myrow["Track"])) . "\n");
	    	PrintIMR("$myrow[GameIP]\n");
	    	PrintIMR("$myrow[Port] $myrow[Laps] $myrow[Weapons] $playercount\n");
	    	// WRITES THE PLAYER'S ID'S INTO SOME FORM OF ARRAY FOR THE PLAYERLIST
			PrintIMR("$playerlist \n");

	    	if($myrow["UserID"]==$UserID AND $myrow["Checked"]=="0")
			{
			    $reloadgames = TRUE;
				file_get_contents("http://" . $_SERVER["SERVER_ADDR"] . $_SERVER['PHP_SELF'] . "?=CHECK_GAME%%$myrow[UserID]%%$myrow[GameIP]%%$myrow[Port]%%$myrow[GameID]");
				mysql_query("UPDATE `gamelist` SET `Checked` = '1' WHERE `UserID` = '$myrow[UserID]' AND `GameID` = '$myrow[GameID]'",$db);
				$return = TRUE;
			}
			
			$oldgid = $myrow["GameID"];
	    } while ($myrow = mysql_fetch_array($result));
	}

    for ( $x = $oldgid+1; $x <= $GLOBALS["MAX_GAMES"]; $x++ ) PrintIMR("GAME $x DEL\n");
    
	return $return;
}

function PlayerListUpdate()
{
	$db = $GLOBALS["db"];
    // CHECKS IF ANY OF THE CLIENT SLOTS ARE TAKEN
	$result = mysql_query("SELECT * FROM `userlist` WHERE `TimeStamp` > '$GLOBALS[TIMEOUT]' ORDER BY `UserID`",$db);
	if($myrow = mysql_fetch_array($result))
	{
		do {
		    if ($myrow["UserID"]-$oldid > 1)
		    {
		        for ( $x = $oldid+1; $x < $myrow["UserID"]; $x++ ) PrintIMR("USER $x DEL\n");
		    }
			// TELLS THE IMR TO ADD THIS CLIENT TO THE PLAYER LIST
	        PrintIMR("USER $myrow[UserID] NEW\n");
	        PrintIMR("\n");
	        PrintIMR("($myrow[UserID]) ");
	        PrintIMR(urldecode($myrow["UserName"]));
	        if ($myrow["Status"]==TRUE) {
	        	PrintIMR(" [" . urldecode($myrow[Status]) . "]");
	        }
			PrintIMR("\n");

			$oldid = $myrow["UserID"];
		} while ($myrow = mysql_fetch_array($result));
	}

	for ( $x = $oldid+1; $x <= $GLOBALS["MAX_PLAYERS"]; $x++ ) PrintIMR("USER $x DEL\n");
}

function ChatMessage($UserID, $UserName, $Message, $ToUser = FALSE, $ToUserName = FALSE)
{
	// if(GetUserAgent() == "1.23") $Message = url_encode($Message);
    mysql_query("INSERT INTO `chat` (`UserID`,`UserName`,`Message`,`TimeStamp`,`ToUser`,`ToUserName`) VALUES ('$UserID','$UserName','" . $Message . "','". time() ."','$ToUser','$ToUserName')",$GLOBALS["db"]);
}
?>
