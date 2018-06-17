<?php
class DataAccess{

	private static $_instance;
	private  $_conn = NULL;
	const HOLDS = 1;
	const NOTHOLDS = 0;
	const UNKNOWN = -1;
	public function __construct($h,$u,$p,$d){
		$mysqli = new mysqli($h,$u,$p,$d);
		if ($mysqli->connect_errno) {
			throw new Exception("Failed to connect to MySQL: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error);
		}
		$this->_conn = $mysqli;
	}

	public function streamContext($contextId, $callback){
		$cbo = NULL;
		$cbm = NULL;
		if(is_array($callback)){
			if(is_object($callback[0])){
				$o = $callback[0];
				$m = $callback[1];
				if(method_exists($o, $m)){
					$cbo = $o;
					$cbm = $m;
				}
			}
		}
		if(!$cbo || !$cbm){
			throw new Exception("Bad callback");
		}
		$mysqli = $this->_conn;
		$q = 'SELECT
				OITEM.NAME AS OBJECT,
				AITEM.NAME AS ATTRIBUTE,
				HOLDS
				FROM
				RELATION,
				ITEM AS OITEM,
				ITEM AS AITEM
				WHERE
				CONTEXT = ? AND RELATION.OBJECT = OITEM.ID AND RELATION.ATTRIBUTE = AITEM.ID
				';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		$stmt->bind_param("i", $contextId);
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if(!$stmt->bind_result($obj, $attr, $holds)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		while($stmt->fetch()){
			$cbo->$cbm($obj, $attr, $holds);
		}
		$stmt->close();
	}

	public function insertType($type){
		$mysqli = $this->_conn;
		$q = 'INSERT INTO TYPE (NAME) VALUES (?)';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("s",$type)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$id = $stmt->insert_id;
		$stmt->close();
		return $id;
	}

	public function getTypeName($typeId){
		$mysqli = $this->_conn;
		$q = 'SELECT NAME FROM TYPE WHERE ID=? LIMIT 1';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		$stmt->bind_param("i", $typeId);
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if(!$stmt->bind_result($name)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->fetch();
		$stmt->close();
		return $name;
	}

	public function typeNames(){
		$mysqli = $this->_conn;
		$q = 'SELECT NAME FROM TYPE ORDER BY NAME';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if(!$stmt->bind_result($name)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		while ($stmt->fetch())
		{
			array_push($items, $name);
		}
		$stmt->close();
		return $items;
	}

	/**
	 * If userid is specified, then a 'writable' field is returned,
	 * otherwise, behaves the same as <code>listTypes()</code>
	 * 
	 * @param string $userid
	 * @throws Exception
	 * @return array
	 */
	public function types($userid = NULL){
		$mysqli = $this->_conn;
		$q = 'SELECT ID, NAME, NULL FROM TYPE ORDER BY ID';
		if(is_int($userid)){
			$q = <<<QUERY
			SELECT TYPE.ID, TYPE.NAME, (P.ID IS NOT NULL) AS WRITABLE 
					FROM TYPE 
					LEFT JOIN TYPE_PERMISSION AS P 
					ON 	P.PERMISSION=2 AND 
						TYPE.ID=P.TYPE AND 
						P.ROLE IN (SELECT R.ROLE FROM ROLE_USER AS R WHERE R.USER=$userid)
					ORDER BY TYPE.ID
QUERY;

		}
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if(!$stmt->bind_result($id, $name, $writable)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		while ($stmt->fetch()){
			$item = array('id' => $id, 'name' => $name);
			if(intval($userid)){
				$item['writable'] = $writable;
			}
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function typeIds(){
		$mysqli = $this->_conn;
		$q = 'SELECT ID FROM TYPE ORDER BY ID';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if(!$stmt->bind_result($id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		while ($stmt->fetch())
		{
			array_push($items, $id);
		}
		$stmt->close();
		return $items;
	}

	public function getContextName($contextId){
		$mysqli = $this->_conn;
		$q = 'SELECT NAME FROM CONTEXT WHERE ID=? LIMIT 1';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		$stmt->bind_param("i", $contextId);
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if(!$stmt->bind_result($name)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->fetch();
		$stmt->close();
		return $name;
	}

	// 	public function getTypeId($type){
	// 		$mysqli = $this->_conn;
	// 		$q = 'SELECT ID FROM TYPE WHERE NAME=? LIMIT 1';
	// 		if (!($stmt = $mysqli->prepare($q))){
	// 			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
	// 		}
	// 		if(!$stmt->bind_param("s",$type)){
	// 			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
	// 		}
	// 		if (!$stmt->execute()) {
	// 			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
	// 		}
	// 		$stmt->bind_result($id);
	// 		if(!$stmt->fetch()){
	// 			$id = FALSE;
	// 		}
	// 		$stmt->close();
	// 		return $id;
	// 	}

	public function getItemIdByName($typeId, $itemName){
		$mysqli = $this->_conn;
		$q = 'SELECT ID FROM ITEM WHERE TYPE=? AND NAME=? LIMIT 1';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		$intId = intval($typeId);
		if(!$stmt->bind_param("is",$intId, $itemName)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($id);
		if(!$stmt->fetch()){
			$id = FALSE;
		}
		$stmt->close();
		return $id;
	}
	
	public function getContextIdByName($contextName){
		$mysqli = $this->_conn;
		$q = 'SELECT ID FROM CONTEXT WHERE NAME=? LIMIT 1';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("s", $contextName)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($id);
		if(!$stmt->fetch()){
			$id = FALSE;
		}
		$stmt->close();
		return $id;
	}

	// 	public function getContextId($context){
	// 		$mysqli = $this->_conn;
	// 		$q = 'SELECT ID FROM CONTEXT WHERE NAME=? LIMIT 1';
	// 		if (!($stmt = $mysqli->prepare($q))){
	// 			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
	// 		}
	// 		$stmt->bind_param("s",$context);
	// 		if (!$stmt->execute()) {
	// 			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
	// 		}
	// 		if(!$stmt->bind_result($id)){
	// 			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
	// 		}
	// 		$row = $stmt->fetch();
	// 		$stmt->close();
	// 		return $id;
	// 	}

	public  function insertItems($typeId, array $items){
		$mysqli = $this->_conn;
		$q = "INSERT IGNORE INTO ITEM (NAME, TYPE) VALUES (?,?);";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		foreach($items as $item){
			if(!$stmt->bind_param('si',$item, $typeId)){
				throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
			}
			if (!$stmt->execute()) {
				throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
			}
		}
		$stmt->close();
	}

	public  function insertItem($typeId, $itemName, $updateContext = TRUE){
		$mysqli = $this->_conn;
		$q = "INSERT IGNORE INTO ITEM (NAME, TYPE) VALUES (?,?);";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param('si',$itemName, $typeId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$id = $mysqli->insert_id;
		$stmt->close();
		if($id && $updateContext){
			$this->updateContextsRelations($typeId);
		}
		return $id;
	}

	public function createContext($name, $description, $objectsId, $attributesId){
		// Insert context
		$contextId = $this->insertContext($name, $description, $objectsId, $attributesId);
		// Build relations
		$this->buildRelations($contextId);
		return $contextId;
	}
	
	public function saveContext($id, $name, $description){
		$mysqli = $this->_conn;
		$q = "UPDATE CONTEXT SET NAME=?, DESCRIPTION=? WHERE ID=?";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param('ssi',$name, $description, $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return TRUE;
	}

	public function updateContextsRelations($typeId){
		$mysqli = $this->_conn;
		$q = "INSERT IGNORE INTO RELATION (OBJECT, ATTRIBUTE, CONTEXT)
				SELECT OBJECT.ID, ATTRIBUTE.ID, CONTEXT.ID
				FROM
				CONTEXT,
				ITEM AS OBJECT,
				ITEM AS ATTRIBUTE
				WHERE
				OBJECT.TYPE = CONTEXT.OBJECTS AND
				ATTRIBUTE.TYPE = CONTEXT.ATTRIBUTES AND
				(CONTEXT.OBJECTS = ? OR CONTEXT.ATTRIBUTES = ?);";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("ii", $typeId, $typeId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return true;
	}
	
	public function buildRelations($contextId){
		$mysqli = $this->_conn;
		$q = "INSERT INTO RELATION (OBJECT, ATTRIBUTE, CONTEXT)
				SELECT OBJECT.ID, ATTRIBUTE.ID, CONTEXT.ID
				FROM
				CONTEXT,
				ITEM AS OBJECT,
				ITEM AS ATTRIBUTE
				WHERE
				CONTEXT.ID = ? AND
				OBJECT.TYPE = CONTEXT.OBJECTS AND
				ATTRIBUTE.TYPE = CONTEXT.ATTRIBUTES;";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return true;
	}

	public function objectsInContext($contextId){
		$q = 'SELECT DISTINCT OBJECT FROM RELATION WHERE CONTEXT = ?';
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id);
		while ($stmt->fetch())
		{
			array_push($items, $id);
		}
		return $items;
	}

	public function attributesInContext($contextId){
		$q = 'SELECT DISTINCT ATTRIBUTE FROM RELATION WHERE CONTEXT = ?';
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id);
		while ($stmt->fetch())
		{
			array_push($items, $id);
		}
		return $items;
	}
	
	public  function insertContext($context, $description, $objectsId, $attributesId){
		$mysqli = $this->_conn;
		$q = "INSERT INTO CONTEXT (NAME, DESCRIPTION, OBJECTS, ATTRIBUTES) VALUES (?,?,?,?);";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param('ssii',$context, $description, $objectsId, $attributesId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$id = $stmt->insert_id;
		$stmt->close();
		return $id;
	}

	public function itemsIds($typeId){
		$mysqli = $this->_conn;
		$q = 'SELECT ITEM.ID FROM ITEM, TYPE WHERE TYPE = TYPE.ID AND TYPE.ID = ?';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("s",$typeId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id);
		while ($stmt->fetch())
		{
			array_push($items, $id);
		}
		return $items;
	}

	public function objectsNames($contextId, array $itemsIds = array()){
		$mysqli = $this->_conn;
		$q = 'SELECT ITEM.NAME FROM ITEM, CONTEXT WHERE TYPE = CONTEXT.OBJECTS AND CONTEXT.ID = ?';
		if(count($itemsIds)>0){
			$q .= ' AND ITEM.ID IN ('. implode(',', $itemsIds) . ') ';
		}
		$q .= ' ORDER BY ITEM.NAME';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($name);
		while ($stmt->fetch())
		{
			array_push($items, $name);
		}
		$stmt->close();
		return $items;
	}

	public function attributesNames($contextId, array $itemsIds = array()){
		$mysqli = $this->_conn;
		$q = 'SELECT ITEM.NAME FROM ITEM, CONTEXT WHERE TYPE = CONTEXT.ATTRIBUTES AND CONTEXT.ID = ? ';
		if(count($itemsIds)>0){
			$q .= ' AND ITEM.ID IN ('. implode(',', $itemsIds) . ') ';
		}
		$q .= ' ORDER BY ITEM.NAME';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($name);
		while ($stmt->fetch())
		{
			array_push($items, $name);
		}
		$stmt->close();
		return $items;
	}

	public function listItems($typeId){
		$mysqli = $this->_conn;
		$q = 'SELECT ITEM.ID, ITEM.NAME FROM ITEM, TYPE WHERE TYPE = TYPE.ID AND TYPE.ID = ?';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$typeId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name);
		while ($stmt->fetch())
		{
			$item = array('id' => $id, 'name' => $name);
			array_push($items, $item);
		}
		return $items;
	}

	public function listTaxonomies(){
		$mysqli = $this->_conn;
		$q = 'SELECT T.ID, T.NAME, COUNT(C.ID) FROM TAXONOMY AS T LEFT JOIN CONCEPT AS C ON T.ID = C.TAXONOMY GROUP BY 1,2';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $concepts);
		while ($stmt->fetch())
		{
			$item = array('id' => $id, 'name' => $name, 'concepts' => $concepts);
			array_push($items, $item);
		}
		return $items;
	}
	

	public function taxonomies($userid = NULL){
		$mysqli = $this->_conn;
		$q = <<<QUERY
		SELECT T.ID, T.NAME, COUNT(C.ID) , NULL AS WRITABLE
				FROM TAXONOMY AS T LEFT JOIN CONCEPT AS C ON T.ID = C.TAXONOMY 
				GROUP BY 1,2
QUERY;
		
		if($userid !== NULL){
			$q = <<<QUERY
			SELECT T.ID, T.NAME, COUNT(C.ID), P.ID IS NOT NULL AS WRITABLE
				FROM TAXONOMY AS T
					LEFT JOIN CONCEPT AS C ON T.ID = C.TAXONOMY
					LEFT JOIN TAXONOMY_PERMISSION AS P ON 
					P.PERMISSION=2 AND 
					T.ID=P.TAXONOMY AND 
					P.ROLE IN (SELECT R.ROLE FROM ROLE_USER AS R WHERE R.USER=$userid) 
				GROUP BY 1,2
QUERY;
			
		}
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $concepts, $writable);
		while ($stmt->fetch())
		{
			$item = array('id' => $id, 'name' => $name, 'concepts' => $concepts);
			if($userid !== NULL){
				$item['writable'] = $writable;
			}
			array_push($items, $item);
		}
		return $items;
	}

	public function listTypes(){
		$mysqli = $this->_conn;
		$q = 'SELECT ID, NAME FROM TYPE ORDER BY ID';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name);
		while ($stmt->fetch())
		{
			$item = array('id' => $id, 'name' => $name);
			array_push($items, $item);
		}
		return $items;
	}

	public function profile($id, $userid = NULL){
		$mysqli = $this->_conn;
		$q = <<<QUERY
		SELECT E.ID, E.NAME, E.DESCRIPTION, DATA, NULL AS WRITABLE
				FROM EXPORTPROFILE AS E WHERE E.ID = ?
QUERY;
		
		if($userid !== NULL){
			$q = <<<QUERY
			SELECT E.ID, E.NAME, E.DESCRIPTION, DATA, P.ID IS NOT NULL AS WRITABLE
				FROM EXPORTPROFILE AS E
					LEFT JOIN EXPORTPROFILE_PERMISSION AS P ON
					P.PERMISSION=2 AND
					E.ID=P.EXPORTPROFILE AND
					P.ROLE IN (SELECT R.ROLE FROM ROLE_USER AS R WHERE R.USER=$userid)
				WHERE E.ID = ?
				GROUP BY 1,2
QUERY;
		
		}
		
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $description, $data, $writable);
		if ($stmt->fetch())
		{
			$item = array('id' => $id, 'name' => $name, 'description' => $description, 'data' => $data, 'writable' => $writable);
		}
		$stmt->close();
		return $item;
	}
	
	public function listExportprofiles($userid = NULL){
		$mysqli = $this->_conn;
		$q = <<<QUERY
		SELECT E.ID, E.NAME, E.DESCRIPTION, NULL AS WRITABLE
				FROM EXPORTPROFILE AS E ORDER BY E.ID
QUERY;
		
		if($userid !== NULL){
			$q = <<<QUERY
			SELECT E.ID, E.NAME, E.DESCRIPTION, P.ID IS NOT NULL AS WRITABLE
				FROM EXPORTPROFILE AS E
					LEFT JOIN EXPORTPROFILE_PERMISSION AS P ON
					P.PERMISSION=2 AND
					E.ID=P.EXPORTPROFILE AND
					P.ROLE IN (SELECT R.ROLE FROM ROLE_USER AS R WHERE R.USER=$userid)
				GROUP BY 1,2
QUERY;
				
				}
				
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $description, $writable);
		while ($stmt->fetch())
		{
			$item = array('id' => $id, 'name' => $name, 'description' => $description, 'writable' => $writable);
			array_push($items, $item);
		}
		return $items;
	}

	public function updateExportprofile($id, $name, $description, $data){
		$mysqli = $this->_conn;
		$q = <<<UPDATE
		UPDATE EXPORTPROFILE SET NAME = ?, DESCRIPTION = ?, DATA = ? WHERE ID = ?
	
UPDATE;
	
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("sssi",$name, $description, $data, $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		return TRUE;
	}

	public function insertExportprofile($name, $description, $data){
		$mysqli = $this->_conn;
		$q = <<<INS
		INSERT INTO EXPORTPROFILE ( NAME, DESCRIPTION, DATA) VALUES (?,?,?)
	
INS;
	
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("sss",$name, $description, $data)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$id = $stmt->insert_id;
		$stmt->close();
		return $id;
	}
	
	public function deleteExportprofile($id){
		$mysqli = $this->_conn;
		$q = "DELETE FROM EXPORTPROFILE WHERE ID = ?";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		return TRUE;
	}
	
	public function getContext($contextId){
		$mysqli = $this->_conn;
		$q = 'SELECT
				CONTEXT.ID AS ID,
				CONTEXT.NAME AS NAME,
				CONTEXT.DESCRIPTION AS DESCRIPTION,
				CONTEXT.OBJECTS AS OBJECTS,
				CONTEXT.ATTRIBUTES AS ATTRIBUTES,
				OTYPE.NAME as OBJECTS_NAME,
				ATYPE.NAME as ATTRIBUTES_NAME
				FROM
				CONTEXT,
				TYPE AS OTYPE,
				TYPE AS ATYPE
				WHERE
				CONTEXT.ID = ? AND
				ATYPE.ID = CONTEXT.ATTRIBUTES AND
				OTYPE.ID = CONTEXT.OBJECTS
				LIMIT 1
				';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}

		$stmt->bind_result($id, $name, $description, $otype, $atype, $otype_name, $atype_name);
		$item = FALSE;
		if($stmt->fetch()){
			$item = array(
					'id' => $id,
					'name' => $name,
					'description' => $description,
					'objects' => $otype,
					'attributes' => $atype,
					'objects_name' => $otype_name,
					'attributes_name' => $atype_name);
		}
		$stmt->close();
		return $item;
	}

	/**
	 * List the contexts. If $type is provided, only contexts using that type as objects or attributes are returned.
	 * 
	 * @param string $type
	 * @throws Exception
	 * @return array
	 */
	public function listContexts($type = NULL){
		$mysqli = $this->_conn;
		$filterType = '';
		if($type != NULL){
			$filterType = ' (ATYPE.ID = ' . $type . ' OR OTYPE.ID = ' . $type . ') AND ';
		}
		$q = 'SELECT
				CONTEXT.ID AS ID,
				CONTEXT.NAME AS NAME,
				CONTEXT.OBJECTS AS OBJECTS,
				CONTEXT.ATTRIBUTES AS ATTRIBUTES,
				OTYPE.NAME as OBJECTS_NAME,
				ATYPE.NAME as ATTRIBUTES_NAME
				FROM
				CONTEXT,
				TYPE AS OTYPE,
				TYPE AS ATYPE
				WHERE
				' . $filterType . '
				ATYPE.ID = CONTEXT.ATTRIBUTES AND
				OTYPE.ID = CONTEXT.OBJECTS
				ORDER BY ID';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $otype, $atype, $otype_name, $atype_name);
		while ($stmt->fetch())
		{
			$item = array(
					'id' => $id,
					'name' => $name,
					'objects' => $otype,
					'attributes' => $atype,
					'objects_name' => $otype_name,
					'attributes_name' => $atype_name);
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}
	
	public function contexts($userid = NULL){
		$mysqli = $this->_conn;
		$filterType = '';
		
		$q = <<<QUERY
			SELECT
				CONTEXT.ID AS ID,
				CONTEXT.NAME AS NAME,
				CONTEXT.OBJECTS AS OBJECTS,
				CONTEXT.ATTRIBUTES AS ATTRIBUTES,
				OTYPE.NAME as OBJECTS_NAME,
				ATYPE.NAME as ATTRIBUTES_NAME,
				NULL as WRITABLE,
				CONTEXT.DESCRIPTION as DESCRIPTION
				FROM
				CONTEXT,
				TYPE AS OTYPE,
				TYPE AS ATYPE
				WHERE
				ATYPE.ID = CONTEXT.ATTRIBUTES AND
				OTYPE.ID = CONTEXT.OBJECTS
				ORDER BY ID
QUERY;
		
		if($userid != NULL){
			$q = <<<QUERY
			SELECT
				CONTEXT.ID AS ID,
				CONTEXT.NAME AS NAME,
				CONTEXT.OBJECTS AS OBJECTS,
				CONTEXT.ATTRIBUTES AS ATTRIBUTES,
				OTYPE.NAME as OBJECTS_NAME,
				ATYPE.NAME as ATTRIBUTES_NAME,
				(P.ID IS NOT NULL) AS WRITABLE,
				CONTEXT.DESCRIPTION as DESCRIPTION
				FROM
				CONTEXT
					LEFT JOIN CONTEXT_PERMISSION AS P ON 
						P.PERMISSION=2 AND 
						CONTEXT.ID=P.CONTEXT AND 
						P.ROLE IN (SELECT R.ROLE FROM ROLE_USER AS R WHERE R.USER=$userid),
				TYPE AS OTYPE,
				TYPE AS ATYPE
				WHERE
				ATYPE.ID = CONTEXT.ATTRIBUTES AND
				OTYPE.ID = CONTEXT.OBJECTS
				ORDER BY ID
QUERY;
		}
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $otype, $atype, $otype_name, $atype_name, $writable, $description);
		while ($stmt->fetch())
		{
			$item = array(
					'id' => $id,
					'name' => $name,
					'objects' => $otype,
					'attributes' => $atype,
					'objects_name' => $otype_name,
					'attributes_name' => $atype_name,
					'description' => $description
			);
			if($userid != NULL){
				$item['writable'] = $writable;
			}
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function countRelations($contextId, $holds = NULL){
		$q = 'SELECT COUNT(ID) FROM RELATION WHERE CONTEXT = ? ';

		if($holds == self::UNKNOWN){
			$q .= ' AND HOLDS IS NULL ';
		}elseif($holds === self::HOLDS){
			$q .= ' AND HOLDS = 1';
		}elseif($holds === self::NOTHOLDS){
			$q .= ' AND HOLDS = 0';
		}

		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($c);
		$ret = FALSE;
		if($stmt->fetch() !== FALSE){
			$ret = $c;
		}
		$stmt->close();
		return $ret;
	}

	public function listAllExtents($contextId){
		$mysqli = $this->_conn;
		$q = 'SELECT
				OBJECT,
				ATTRIBUTE
				FROM
				RELATION
				WHERE
				HOLDS = 1 AND
				CONTEXT = ?
				ORDER BY ATTRIBUTE
				';
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$extents = array();
		$stmt->bind_result($o, $a);
		$ret = FALSE;
		$lastAttribute = NULL;
		$objects = array();
// 		$allObjects = array();
		$first = TRUE;
		while($stmt->fetch()){
// 			if(!in_array( $o, $allObjects)){
// 				array_push($allObjects, $o);
// 			}
			 	//		print $a . '<br/>';
			if($a != $lastAttribute){
// 				print $a . '<br/>';
				sort($objects);
				if(!in_array($objects, $extents)){
					if($first){
						$first = FALSE;
					}else{
						array_push($extents, $objects);
					}
				}
				$objects = array();
			}
			array_push($objects, $o);
			sort($objects);
			$lastAttribute = $a;
		}
		// Add last item in the resultset
		if(!in_array($objects, $extents) && $first != TRUE){
			array_push($extents, $objects);
		}
// 		sort($allObjects);
// 		if(!in_array($allObjects, $extents)){
// 			array_push($extents, $allObjects);
// 		}
		$stmt->close();
		return $extents;
	}

	public function intent($contextId, array $objects = array()){
// 		print 'OBJECTS:<pre>';
// 		print_r($objects);
// 		print '</pre>';
		$mysqli = $this->_conn;
		$q = 'SELECT
				OBJECT,
				ATTRIBUTE
			FROM
				RELATION
			WHERE
				HOLDS = 1 AND
				CONTEXT = ?
				';
		if(count($objects)>0){
			$q .= ' AND OBJECT IN (' . implode(',', $objects) . ')';
		}
		
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$intent = array();
		$stmt->bind_result($o, $a);
		$allAttributes = array();
		while($stmt->fetch()){
			if(!in_array($a, $allAttributes)){
				array_push($allAttributes, $a);
			}
			if(!isset($intent[$o])){
				$intent[$o] = array();
			}
			array_push($intent[$o], $a);
		}
		$stmt->close();
		if(empty($objects)){
			sort($allAttributes);
			return $allAttributes;
		}
		
		$result = NULL;
		if(count(array_values($intent))>1){
			$result = call_user_func_array('array_intersect', array_values($intent));
		}else if(count(array_values($intent))==1){
			$avs = array_values($intent);
			$result = array_pop($avs);
			unset($avs);
		}else{
			$result = array();
		}
		sort($result);
// 		print 'INTENT<pre>';
// 		print_r($intent);
// 		print_r($result);
// 		print '</pre>';
		return $result;
	}

	public function extent($contextId, array $attributes = array()){
		$mysqli = $this->_conn;
		$q = 'SELECT
				OBJECT,
				ATTRIBUTE
				FROM
				RELATION
				WHERE
				HOLDS = 1 AND
				CONTEXT = ?
				';
		if(count($attributes)>0){
			$q .= 'AND ATTRIBUTE IN (' . implode(',', $attributes) . ')';
		}
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$extent = array();
		$stmt->bind_result($o, $a);
		$allObjects = array();
		while($stmt->fetch()){
			if(!in_array($o, $allObjects)){
				array_push($allObjects, $o);
			}
			if(!isset($extent[$a])){
				$extent[$a] = array();
			}
			array_push($extent[$a], $o);
		}
		$stmt->close();
		if(empty($attributes)){
			sort($allObjects);
			return $allObjects;
		}
		$result = NULL;
		if(count(array_values($extent))>1){
			$result = call_user_func_array('array_intersect', array_values($extent));
		}else{
			$result = array_pop(array_values($extent));
		}
		sort($result);
		return $result;
	}
	
	public function contextStats($contextId){
		$mysqli = $this->_conn;
		$q = "SELECT COUNT(*) AS NUMBER, HOLDS FROM RELATION WHERE CONTEXT = ? GROUP BY 2";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$contextId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($number, $holds);
		$stats = array();
		$tot = 0;
		while($stmt->fetch()){
			if($holds === NULL){
				$holds = 'null';
			}else{
				$holds = ($holds) ? 'true' : 'false';
			}
			$stats[$holds] = $number;
			$tot = $tot + $number;
		}
		$stmt->close();
		
		$stats['total'] = $tot;
		return $stats;
	}

	public function relations($contextId, $filterObject = NULL, $filterAttribute = NULL, $filterHolds = NULL, $filterObjectHaving = array(), $filterAttributeHaving = array(), $filterObjectMatches = NULL, $filterAttributeMatches = NULL){
		$mysqli = $this->_conn;
		$q = 'SELECT
				RELATION.ID AS ID,
				OBJECT,
				ATTRIBUTE,
				HOLDS,
				OITEM.NAME AS OBJECT_NAME,
				AITEM.NAME AS ATTRIBUTE_NAME
				FROM
				RELATION,
				ITEM AS OITEM,
				ITEM AS AITEM
				WHERE
				CONTEXT = ? AND
				';
		$bindings = array();
		$code = '';
		array_push($bindings, $contextId);
		$code .= 's';
		if($filterObject !== NULL){
			$q .= 'OITEM.ID = ? AND ';
			array_push($bindings, $filterObject);
			$code .= 'i';
		}
		if($filterAttribute !== NULL){
			$q .= 'AITEM.ID = ? AND ';
			array_push($bindings, $filterAttribute);
			$code .= 'i';
		}
		if($filterHolds === 1){
			$q .= 'RELATION.HOLDS = 1 AND ';
		}else if($filterHolds === 0){
			$q .= 'RELATION.HOLDS = 0 AND ';
		} else if($filterHolds === -1){
			$q .= 'RELATION.HOLDS IS NULL AND ';
		}
		$q .='  OITEM.ID = RELATION.OBJECT AND
				AITEM.ID = RELATION.ATTRIBUTE ';

		// Only objects having one of the attribute in filterObjectHaving
		$foc = 0;
		foreach($filterObjectHaving as $fov){
			$foc++;
			array_push($bindings, $fov);
			$code .= 'i';
		}
		if($foc > 0){
			$fovs = array_fill(0, $foc, ' FO.ATTRIBUTE = ? ');
			$q .= ' AND OITEM.ID IN (
					SELECT FO.OBJECT FROM RELATION AS FO
					WHERE
					FO.CONTEXT = RELATION.CONTEXT AND
					FO.HOLDS = TRUE AND
					( ' . implode(' OR ', $fovs) . ') )';

		}
		// Only attributes having one of the attribute in filterObjectHaving
		$fac = 0;
		foreach($filterAttributeHaving as $fav){
			$fac++;
			array_push($bindings, $fav);
			$code .= 'i';
		}
		if($fac > 0){
			$favs = array_fill(0, $fac, ' FO.OBJECT = ? ');
			$q .= ' AND AITEM.ID IN (
					SELECT FO.ATTRIBUTE FROM RELATION AS FO
					WHERE
					FO.CONTEXT = RELATION.CONTEXT AND
					FO.HOLDS = TRUE AND
					( ' . implode(' OR ', $favs) . ') )';

		}

		if($filterObjectMatches !== NULL){
			$q .= ' AND OITEM.NAME REGEXP ? ';
			array_push($bindings, $filterObjectMatches);
			$code .= 's';
		}
		if($filterAttributeMatches !== NULL){
			$q .= ' AND AITEM.NAME REGEXP ? ';
			array_push($bindings, $filterAttributeMatches);
			$code .= 's';
		}
		
		$q .= ' ORDER BY RELATION.ID ';

		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		$bound = FALSE;
		array_unshift($bindings, $code);

		$bound = call_user_func_array(array($stmt, "bind_param"), self::refValues($bindings));

		if(!$bound){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $object, $attribute, $holds, $object_name, $attribute_name);
		while ($stmt->fetch())
		{
			$item = array(
					'id' => $id,
					'object' => $object,
					'attribute' => $attribute,
					'holds' => $holds,
					'object_name' => $object_name,
					'attribute_name' => $attribute_name
			);
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function holds($contextId, $relations, $holds, $userid){
		if(!is_array($relations)){
			$relations = array($relations);
		}
		$relations = implode(',', $relations);
		$mysqli = $this->_conn;
		$mysqli->autocommit(FALSE);
		$e = NULL;
		try{
			$logid = $this->log($userid, $contextId, $relations, $holds);
			$q = "UPDATE RELATION SET HOLDS = ? WHERE ID IN ($relations)";
			if (!($stmt = $mysqli->prepare($q))){
				throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
			}
			if(!$stmt->bind_param('i', $holds)){
				throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
			}
			if (!$stmt->execute()) {
				throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
			}
		}catch(Exception $e){
			$mysqli->rollback();
		}
		$mysqli->commit();
		if(isset($stmt)){
			$stmt->close();
		}
		$mysqli->autocommit(TRUE);
		if($e!==NULL) throw $e;
		return $logid;
	}
	
	public function bulkHolds($contextId, $when = NULL, $then = NULL){
// 		if(!is_array($relations)){
// 			$relations = array($relations);
// 		}
// 		$relations = implode(',', $relations);
		$mysqli = $this->_conn;
		$mysqli->autocommit(FALSE);
		$e = NULL;
		try{
			//$logid = $this->log($userid, $contextId, $relations, $holds);
			
			if($then === NULL){
				$then = "HOLDS = NULL";
			}else{
				$then = "HOLDS = " . (($then) ? "1" : "0");
			}
			
			if($when === NULL){
				$when = "HOLDS IS NULL";
			}else{
				$when = "HOLDS = " . (($when) ? "1" : "0");
			}
			$q = "UPDATE RELATION SET $then WHERE CONTEXT = ? AND $when";
				
			if (!($stmt = $mysqli->prepare($q))){
				throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
			}
			if(!$stmt->bind_param('i', $contextId)){
				throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
			}
			if (!$stmt->execute()) {
				throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
			}
		}catch(Exception $e){
			$mysqli->rollback();
		}
		$mysqli->commit();
		if(isset($stmt) && is_object($stmt)){
			$stmt->close();
		}
		$mysqli->autocommit(TRUE);
		if($e!==NULL) throw $e;
		return TRUE;
	}

	public function unholds($contextId, $relations, $userid){
		if(!is_array($relations)){
			$relations = array($relations);
		}
		$relations = implode(',', $relations);
		$mysqli = $this->_conn;
		$mysqli->autocommit(FALSE);
		$e = NULL;
		try{
			$logid = $this->log($userid, $contextId, $relations, NULL);
			$q = "UPDATE RELATION SET HOLDS = NULL WHERE ID IN ($relations)";
			if (!($stmt = $mysqli->prepare($q))){
				throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
			}
			if (!$stmt->execute()) {
				throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
			}
		}catch(Exception $e){
			$mysqli->rollback();
		}
		$mysqli->commit();
		if(isset($stmt)){
			$stmt->close();
		}
		$mysqli->autocommit(TRUE);
		if($e!==NULL) throw $e;
		return $logid;
	}

	public function log($userid, $context, $relations, $holds){
		if($holds === NULL) $holds = 'NULL';
		$q = "INSERT INTO ACTION_LOG (CONTEXT, USER) VALUES (?,?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param('ii', $context, $userid)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$actionId = $mysqli->insert_id;
		$q = "INSERT INTO CHANGE_LOG (ACTION, CHANGE_LOG.RELATION, CHANGE_LOG.WAS, CHANGE_LOG.IS)
		SELECT $actionId, ID, HOLDS, $holds FROM RELATION WHERE ID IN ($relations)";
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("$q\nPrepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		return $actionId;
	}

	public function revertChange($context, $logactionid,  $userid){
		$mysqli = $this->_conn;
		$mysqli->autocommit(FALSE);
		$e = NULL;
		try{
			$q = "INSERT INTO ACTION_LOG (CONTEXT, USER) VALUES (?,?)";
			$mysqli = $this->_conn;
			if (!($stmt = $mysqli->prepare($q))){
				throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
			}
			if(!$stmt->bind_param('ii', $context, $userid)){
				throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
			}
			if (!$stmt->execute()) {
				throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
			}
			$actionId = $mysqli->insert_id;

			$q = "INSERT INTO CHANGE_LOG (ACTION, CHANGE_LOG.RELATION, CHANGE_LOG.WAS, CHANGE_LOG.IS)
			SELECT $actionId, RELATION.ID, RELATION.HOLDS, WAS.WAS FROM
			RELATION,
			CHANGE_LOG AS WAS
			WHERE RELATION.ID = WAS.RELATION AND WAS.ACTION = ?";
			if (!($stmt = $mysqli->prepare($q))){
				throw new Exception("$q\nPrepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
			}
			if(!$stmt->bind_param('i', $logactionid)){
				throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
			}
			if (!$stmt->execute()) {
				throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
			}

			$q = "UPDATE RELATION
			INNER JOIN CHANGE_LOG ON RELATION.ID = CHANGE_LOG.RELATION
			SET HOLDS = CHANGE_LOG.WAS WHERE CHANGE_LOG.ACTION = $logactionid";
			if (!($stmt = $mysqli->prepare($q))){
				throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
			}
			if (!$stmt->execute()) {
				throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
			}
		}catch(Exception $e){
			$mysqli->rollback();
		}
		$mysqli->commit();
		if(isset($stmt) && $stmt!==FALSE){
			$stmt->close();
		}
		$mysqli->autocommit(TRUE);
		if($e!==NULL) throw $e;
	}
	public function readLogAction($logactionid){
		$q = "SELECT
				ACTION_LOG.ID,
				ACTION_LOG.CONTEXT,
				ACTION_LOG.USER,
				USER.USERNAME,
				ACTION_LOG.TIME,
				COUNT(CHANGE_LOG.ACTION)
				FROM
				ACTION_LOG,
				CHANGE_LOG,
				USER
				WHERE
				ACTION_LOG.ID = ? AND
				ACTION_LOG.ID = CHANGE_LOG.ACTION AND
				USER.ID = ACTION_LOG.USER
				LIMIT 1";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param('i', $logactionid)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}

		$stmt->bind_result($id, $context, $userid, $username, $time, $changes);
		if ($stmt->fetch())
		{
			$item = array(
					'id' => $id,
					'context' => $context,
					'userid' => $userid,
					'username' => $username,
					'time' => $time,
					'changes' => $changes
			);

		}
		$stmt->close();
		return $item;
	}

	public function readLogChanges($logactionid){
		$q = "SELECT
				CHANGE_LOG.RELATION,
				CHANGE_LOG.WAS,
				CHANGE_LOG.IS,
				RELATION.OBJECT,
				RELATION.ATTRIBUTE,
				OITEM.NAME,
				AITEM.NAME
				FROM
				CHANGE_LOG,
				RELATION,
				ITEM AS OITEM,
				ITEM AS AITEM
				WHERE
				CHANGE_LOG.ACTION = ? AND
				CHANGE_LOG.RELATION = RELATION.ID AND
				OITEM.ID = RELATION.OBJECT AND
				AITEM.ID = RELATION.ATTRIBUTE
				";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param('i', $logactionid)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}

		$stmt->bind_result($id, $was, $is, $object, $attribute, $object_name, $attribute_name);
		$items = array();
		while ($stmt->fetch())
		{

			$item = array(
					'id' => $id,
					'was' => $was,
					'is' => $is,
					'object' => $object,
					'attribute' => $attribute,
					'object_name' => $object_name,
					'attribute_name' => $attribute_name
			);
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function readLog($limit = 100, $context = ''){

		if($context !== ''){
			$context = ' AND ACTION_LOG.CONTEXT = ' . $context;
		}
		$q = "SELECT
		ACTION_LOG.ID,
		ACTION_LOG.CONTEXT,
		ACTION_LOG.USER,
		USER.USERNAME,
		ACTION_LOG.TIME,
		COUNT(CHANGE_LOG.ACTION)
		FROM
		ACTION_LOG,
		CHANGE_LOG,
		USER
		WHERE
		ACTION_LOG.ID = CHANGE_LOG.ACTION AND
		USER.ID = ACTION_LOG.USER $context
		GROUP BY
		ACTION_LOG.ID,
		ACTION_LOG.CONTEXT,
		ACTION_LOG.USER,
		USER.USERNAME,
		ACTION_LOG.TIME
		ORDER BY ACTION_LOG.TIME DESC
		LIMIT $limit;";

		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $context, $userid, $username, $time, $changes);
		while ($stmt->fetch())
		{
			$item = array(
					'id' => $id,
					'context' => $context,
					'userid' => $userid,
					'username' => $username,
					'time' => $time,
					'changes' => $changes
			);
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function user($userid){
		$q = "SELECT ID, USERNAME FROM USER WHERE ID = (?) LIMIT 1";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i",$userid)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($id, $username);
		$ret = $stmt->fetch();
		if($ret !== FALSE){
			$ret = array('id' => $id, 'name' => $username);
		}
		$stmt->close();
		return $ret;
	}
	
	public function userByName($user){
		$q = "SELECT ID, USERNAME FROM USER WHERE NAME = (?) LIMIT 1";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("s",$user)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($id, $username);
		$ret = $stmt->fetch();
		if($ret !== FALSE){
			$ret = array('id' => $id, 'name' => $username);
		}
		$stmt->close();
		return $ret;
	}
	
	public function insertUser($username, $password){
		$da = DataAccess::instance();
		$mysqli = DataAccess::connection ();
		$q = 'INSERT INTO USER (USERNAME,PASSWORD) VALUES (?,?);';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "ss", $username, $password )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$userid = $mysqli->insert_id;
		$stmt->close ();
		return $userid;
	}
	
	public function createUser($username, $password){
		$da = DataAccess::instance();
	 	$userid = $this->insertUser($username, $password);
	 	// User is active
	 	ACL::addToRole($userid, 1);
	 	$roleid = ACL::createRole($username, 'Role for user ' . $username);
	 	ACL::addToRole($userid, $roleid);
	}

	public function login($username, $password){
		$q = "SELECT ID FROM USER WHERE USERNAME=? AND PASSWORD=? LIMIT 1";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("ss",$username,$password)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($c);
		$ret = FALSE;
		if($stmt->fetch() !== FALSE){
			$ret = $c;
		}
		$stmt->close();
		return $ret;
	}

	/**
	 *
	 * @param string $file
	 * @param string $contextName
	 * @param string $objectTypeName
	 * @param string|NULL $attributeTypeName - If NULL is the same type of the objects
	 */
	public function importContextFromCsv($file, $contextName, $objectTypeName, $attributeTypeName = NULL, $fillAs = NULL){
		#print '#1 ' . $file . ' - ' . $contextName . ' - ' . $objectTypeName . ' ' . $attributeTypeName . '<br/>';
		$contextId = FALSE;
		$mysqli = $this->_conn;
		$mysqli->autocommit(FALSE);
		$e = NULL;
		try{
			// Create object type
			$objectTypeId = $this->insertType($objectTypeName);
			if($attributeTypeName !== NULL){
				$attributeTypeId = $this->insertType($attributeTypeName);
			}else{
				$attributeTypeId = $objectTypeId;
			}
			// Create context
			$contextId = $this->createContext($contextName, "", $objectTypeId, $attributeTypeId);
			#print '#2 ' . $contextName . ' ' . $objectTypeId . ' ' . $attributeTypeId . '<br/>';
			$handle = fopen($file, "r");
			if ($handle) {
				try{
					$q = "INSERT IGNORE INTO RELATION (OBJECT, ATTRIBUTE, CONTEXT, HOLDS) VALUES (?,?,?,?)";
					if (!($stmt = $mysqli->prepare($q))){
						throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
					}
					$q2 = "INSERT INTO ITEM (NAME, TYPE) VALUES (?,?);";
					if (!($item_stmt = $mysqli->prepare($q2))){
						throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
					}
					while (($line = fgetcsv($handle)) !== false) {
						//$line = explode(',', $line);
						$o = $line[0];
						$a = $line[1];
						$h = $line[2];
						switch ( $h == '1' ){
							case '1':
								break;
							case '0';
							break;
							break;
							default:
								$h = NULL;
						}
						// Be sure items are inserted and get their id
						$oId = $this->getItemIdByName($objectTypeId, $o);
						if($oId == FALSE){
							$oId = $this->insertItem($objectTypeId, $o, FALSE);
						}
						$aId = $this->getItemIdByName($attributeTypeId, $a);
						if($aId == FALSE){
							$aId = $this->insertItem($attributeTypeId, $a, FALSE);
						}
						if(!$stmt->bind_param("iiii", $oId, $aId, $contextId, $h)){
							throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
						}
						
						if (!$stmt->execute()) {
							throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error );
						}
					}
					// Update context by including non mentioned associations between objects/attributes
					$this->updateContextsRelations($objectTypeId);
					$this->updateContextsRelations($attributeTypeId);
				}catch(Exception $ex){
					fclose($handle);
					throw $ex;
				}
				fclose($handle);
			}else {
				throw Exception('Cannot read CSV');
			}
		}catch(Exception $e){
			$mysqli->rollback();
		}
		$mysqli->commit();
		if(isset($stmt)){
			$stmt->close();
		}
		if(isset($item_stmt)){
			$item_stmt->close();
		}
		$mysqli->autocommit(TRUE);
		if($e!==NULL) throw $e;

		return $contextId;
	}

	public function insertTaxonomy($name){
		$q = "INSERT INTO TAXONOMY (NAME) VALUES (?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("s",$name)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$taxonomyId = $mysqli->insert_id;
		$stmt->close();
		return $taxonomyId;
	}

	public function createConcept($taxonomyId, $extent, $intent){
		$hash = md5(serialize(array($taxonomyId, $extent, $intent)), TRUE);
		$q = "INSERT IGNORE INTO CONCEPT (HASH, TAXONOMY, EXTENT, INTENT) VALUES (?,?,?,?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("siss", $hash, $taxonomyId, $extent, $intent)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$conceptId = $mysqli->insert_id;
		$stmt->close();
		if($conceptId == 0){
			$conceptId = $this->getConceptIdByHash($hash);
		}
		return $conceptId;
	}

	public function getConceptIdByHash($hash){
		$q = "SELECT ID FROM CONCEPT WHERE HASH = ?";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("s", $hash)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($conceptId);
		if(!$stmt->fetch()){
			$conceptId = false;
		}
		$stmt->close();
		return $conceptId;
	}

	public function insertSubconcept($subconceptId, $parentId, $taxonomyId){
		$q = "INSERT IGNORE INTO SUBCONCEPT (CONCEPT, PARENT, TAXONOMY) VALUES (?,?,?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("iii", $subconceptId, $parentId, $taxonomyId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
	}

	public function latticeTop($taxonomyId){
		$q = "SELECT DISTINCT PARENT AS ROOT FROM SUBCONCEPT WHERE TAXONOMY = ?
				AND PARENT NOT IN (SELECT B.CONCEPT FROM SUBCONCEPT AS B WHERE TAXONOMY = ?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("ii", $taxonomyId, $taxonomyId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($item);
		while ($stmt->fetch())
		{
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function latticeBottom($taxonomyId){
		$q = "SELECT DISTINCT CONCEPT AS ROOT FROM SUBCONCEPT WHERE TAXONOMY = ?
				AND CONCEPT NOT IN (SELECT B.PARENT FROM SUBCONCEPT AS B WHERE TAXONOMY = ?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("ii", $taxonomyId, $taxonomyId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($item);
		while ($stmt->fetch())
		{
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function taxonomyChildren($parentId){
		$q = "SELECT CONCEPT FROM SUBCONCEPT WHERE
				PARENT = ?";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $parentId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($item);
		while ($stmt->fetch())
		{
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function parents($taxonomyId){
		$q = "SELECT CONCEPT, PARENT FROM SUBCONCEPT WHERE
				TAXONOMY = ?";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $taxonomyId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($concept, $parent);
		while ($stmt->fetch())
		{
			if(!isset($items[$concept])){
				$items[$concept] = array();
			}
			array_push($items[$concept], $parent);
		}
		$stmt->close();
		return $items;
	}

	public function children($taxonomyId){
		$q = "SELECT PARENT, CONCEPT FROM SUBCONCEPT WHERE
				TAXONOMY = ?";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $taxonomyId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($parent, $child);
		while ($stmt->fetch())
		{
			if(!isset($items[$parent])){
				$items[$parent] = array();
			}
			array_push($items[$parent], $child);
		}
		$stmt->close();
		return $items;
	}
	

	public function taxonomyChildrenData($conceptId){
		$q = "SELECT ID, NAME, COMMENT, EXTENT, INTENT FROM CONCEPT WHERE ID IN (SELECT CONCEPT FROM SUBCONCEPT WHERE
				PARENT = ?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $conceptId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $comment, $extent, $intent);
		while ($stmt->fetch())
		{
			$item = array(
				'id' => $id,
				'name' => $name,
				'comment' => $comment,
				'extent' => $extent,
				'intent' => $intent
			);
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}
	
	public function taxonomy($taxonomyId){
		$q = 'SELECT NAME FROM TAXONOMY WHERE ID = ?';
		
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $taxonomyId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->bind_result($name);
		$taxonomy = FALSE;	
		if ($stmt->fetch()){
			$taxonomy = array('id' => $taxonomyId, 'name' => $name);
		}
		$stmt->close();
		return $taxonomy;
	}
	
	/**
	 * 
	 * @param int $taxonomyId
	 * @param boolean $sort - When true order by length of the extent in ascending order, false otherwise 
	 * @throws Exception
	 * @return multitype:
	 */
public function concepts($taxonomyId, $sort = true){
		$q = "SELECT
				ID,
				NAME,
				COMMENT,
				EXTENT,
				INTENT
				FROM
				CONCEPT
				WHERE
				TAXONOMY = ?";
		$q = $q . " order by char_length(EXTENT) " . ($sort ? 'ASC' : 'DESC') ;
		
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $taxonomyId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->store_result();

		$stmt->bind_result($id, $name, $comment, $extent, $intent);
		$items = array();
		while ($stmt->fetch()){
			$concept = array(
					'id' => $id,
					'name' => $name,
					'comment' => $comment,
					'extent' => $extent,
					'intent' => $intent
			);
				
			$concept['top'] = $this->taxonomyParents($id);
			$concept['bottom'] = $this->taxonomyChildren($id);
			array_push($items, $concept);
		}
		$stmt->free_result();
		$stmt->close();
		return $items;
	}
	
	public function concept($conceptId){
		$q = "SELECT
				ID,
				NAME,
				COMMENT,
				EXTENT,
				INTENT
				FROM
				CONCEPT
				WHERE
				ID = ? LIMIT 1";
	
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $conceptId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->store_result();
		$stmt->bind_result($id, $name, $comment, $extent, $intent);
		$concept = array();
		if ($stmt->fetch()){
			$concept = array(
					'id' => $id,
					'name' => $name,
					'comment' => $comment,
					'extent' => $extent,
					'intent' => $intent
			);
			$concept['top'] = $this->taxonomyParentsData($id);
			$concept['bottom'] = $this->taxonomyChildrenData($id);
			
		}
		$stmt->free_result();
		$stmt->close();
		return $concept;
	}

	public function taxonomyParents($conceptId){
		$q = "SELECT PARENT FROM SUBCONCEPT WHERE
				CONCEPT = ?";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $conceptId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($item);
		while ($stmt->fetch())
		{
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}
	
	public function taxonomyParentsData($conceptId){
		$q = "SELECT ID, NAME, COMMENT, EXTENT, INTENT FROM CONCEPT WHERE ID IN (SELECT PARENT FROM SUBCONCEPT WHERE
				CONCEPT = ?)";
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $conceptId)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$items = array();
		$stmt->bind_result($id, $name, $comment, $extent, $intent);
		while ($stmt->fetch())
		{
			$item = array(
				'id' => $id,
				'name' => $name,
				'comment' => $comment,
				'extent' => $extent,
				'intent' => $intent
			);
			array_push($items, $item);
		}
		$stmt->close();
		return $items;
	}

	public function createTaxonomy($contextId, array $taxonomy){
		$taxonomyId = FALSE;
		$mysqli = $this->_conn;
		$mysqli->autocommit(FALSE);
		$e = NULL;
		try{
			$name = $this->getContextName($contextId);
			$taxonomyId = $this->insertTaxonomy($name);
			foreach($taxonomy as $subrel){
				$concept1 = $subrel[0];
				if(count($concept1[0]) > 0){
				$extent = $this->objectsNames($contextId, $concept1[0]);
				}else $extent = array();
				if(count($concept1[1]) > 0){
				$intent = $this->attributesNames($contextId, $concept1[1]);
				}else $intent = array();
				$extent = implode(',', $extent);
				$intent = implode(',', $intent);
				$concept1Id = $this->createConcept($taxonomyId, $extent, $intent);

				$concept2 = $subrel[1];
				if(count($concept2[0]) > 0){
					$extent = $this->objectsNames($contextId, $concept2[0]);
				}else $extent = array();
				if(count($concept2[1]) > 0){
					$intent = $this->attributesNames($contextId, $concept2[1]);
				}else $intent = array();
				
				$extent = implode(',', $extent);
				$intent = implode(',', $intent);
				$concept2Id = $this->createConcept($taxonomyId, $extent, $intent);
				$this->insertSubconcept($concept1Id, $concept2Id, $taxonomyId);
			}
		}catch(Exception $e){
			$mysqli->rollback();
		}
		$mysqli->commit();
		if(isset($stmt)){
			$stmt->close();
		}
		if(isset($item_stmt)){
			$item_stmt->close();
		}
		$mysqli->autocommit(TRUE);
		if($e!==NULL) throw $e;
		return $taxonomyId;
	}

	public function updateConcept($id, $name, $comment){
		$q = 'UPDATE CONCEPT SET NAME=?, COMMENT=? WHERE ID=?;';
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("ssi", $name, $comment, $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return TRUE;
	}

public function deleteConcept($id){
		$q = 'DELETE FROM CONCEPT WHERE ID=?;';
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return TRUE;
	}

	public function deleteContext($id){
		$q = 'DELETE FROM CONTEXT WHERE ID=?;';
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return TRUE;
	}

	public function deleteTaxonomy($id){
		$q = 'DELETE FROM TAXONOMY WHERE ID=?;';
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return TRUE;
	}
	
	public function deleteType($id){
		$q = 'DELETE FROM TYPE WHERE ID=?;';
		$mysqli = $this->_conn;
		if (!($stmt = $mysqli->prepare($q))){
			throw new Exception("Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error);
		}
		if(!$stmt->bind_param("i", $id)){
			throw new Exception("Bind failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		if (!$stmt->execute()) {
			throw new Exception("Execute failed: (" . $stmt->errno . ") " . $stmt->error);
		}
		$stmt->close();
		return TRUE;
	}
	
	public static function refValues($arr){
		if (strnatcmp(phpversion(),'5.3') >= 0) //Reference is required for PHP 5.3+
		{
			$refs = array();
			foreach($arr as $key => $value)
				$refs[$key] = &$arr[$key];
			return $refs;
		}
		return $arr;
	}
	
	public static function connection(){
		$me = self::instance();
		return $me->_conn;
	}
	
	public static function instance(){
		if(self::$_instance === NULL){
			$ini = parse_ini_file(dirname(__FILE__) . '/dataaccess.ini');
			//"localhost", "root", "root", "contento"
			self::$_instance = new DataAccess($ini['host'], $ini['user'], $ini['password'], $ini['db']);
		}
		return self::$_instance;
	}
}
