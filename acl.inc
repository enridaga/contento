<?php
class ACL {
	const TYPE = 'TYPE';
	const CONTEXT = 'CONTEXT';
	const TAXONOMY = 'TAXONOMY';
	const EXPORTPROFILE = 'EXPORTPROFILE';

	const P_READ = 1;
	const P_WRITE = 2;
	const P_GRANT = 3;
	
	const R_ACTIVE = 1;
	
	/**
	 * Tests if $userid has $permission on object $object_id of type $object_type
	 * 
	 * @param int $userid
	 * @param int $permission
	 * @param string $object_type
	 * @param int $object_id
	 * @throws Exception
	 * @return boolean
	 */
	public static function allowed($userid, $permission, $object_type, $object_id) {
		$mysqli = DataAccess::connection ();
		$q = 'SELECT ID FROM ' . strtoupper ( $object_type ) . '_PERMISSION WHERE PERMISSION=? AND ' . strtoupper ( $object_type ) . '=? AND ROLE IN ( SELECT ROLE FROM ROLE_USER WHERE USER = '.$userid.' ) LIMIT 1;';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "ii", $permission, $object_id )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->bind_result ( $id );
		$stmt->fetch ();
		$stmt->close ();
		return ($id) ? true : false;
	}

	public static function grant($role, $permission, $object_type, $object_id) {
		$mysqli = DataAccess::connection ();
		$q = 'INSERT IGNORE INTO ' . strtoupper ( $object_type ) . '_PERMISSION (ROLE, PERMISSION, ' . strtoupper ( $object_type ) . ') VALUES (?,?,?);';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "iii", $role, $permission, $object_id )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->close ();
		return TRUE;
	}

	public static function grantUser($userid, $permission, $object_type, $object_id) {
		return self::grant(ACL::userRole($userid), $permission, $object_type, $object_id);
	}
	
	public static function revoke($role, $permission, $object_type, $object_id) {
		$mysqli = DataAccess::connection ();
		$q = 'DELETE FROM ' . strtoupper ( $object_type ) . '_PERMISSION WHERE ROLE=? AND PERMISSION=? AND ' . strtoupper ( $object_type ) . '=? ;';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "iii", $role, $permission, $object_id )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->close ();
		return TRUE;
	}

	public static function createRole($role, $description){
		$da = DataAccess::instance();
		$mysqli = DataAccess::connection ();
		$q = 'INSERT IGNORE INTO ROLE (NAME, DESCRIPTION) VALUES (?,?);';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "ss", $role, $description )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$roleid = $mysqli->insert_id;
		$stmt->close ();
		return $roleid;
	}
	
	public static function addToRole($user, $role){
		$da = DataAccess::instance();
		$userid = $da->userByName($user);
		$mysqli = DataAccess::connection ();
		$q = 'INSERT IGNORE INTO ROLE_USER (ROLE,USER) VALUES (?,?);';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "ii", $role, $userid )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->close ();
		return TRUE;
	}
	
	/**
	 * 
	 * @param int $userid
	 * @param int $role
	 * @throws Exception
	 * @return boolean
	 */
	public static function member($userid, $role){
		$da = DataAccess::instance();
		$mysqli = DataAccess::connection ();
		$q = 'SELECT 1 FROM ROLE_USER WHERE ROLE=? AND USER=? LIMIT 1;';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "ii", $role, $userid )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->bind_result ( $true );
		$stmt->fetch ();
		$stmt->close ();
		return ($true) ? true : false;
	}
	
	public static function removeFromRole($userid, $role){
		$da = DataAccess::instance();
		$mysqli = DataAccess::connection ();
		$q = 'DELETE FROM ROLE_USER WHERE ROLE=? AND USER=?;';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "ii", $role, $userid )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->close ();
		return TRUE;
	}
	
	public static function roles($userid){
		$da = DataAccess::instance();
		$mysqli = DataAccess::connection ();
		$q = 'SELECT ROLE FROM ROLE_USER WHERE USER=?;';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->bind_param ( "ii", $role, $userid )) {
			throw new Exception ( "Bind failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->bind_result($role);
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$roles = array();
		while($this->fetch()){
			array_push($roles);
		}
		$stmt->close ();
		return $roles;
	}
	
	public static function userRole($userid){
		$da = DataAccess::instance();
		$mysqli = DataAccess::connection ();
		$q = 'SELECT ID FROM ROLE WHERE NAME IN (SELECT USERNAME FROM USER WHERE ID=' . $userid . ') LIMIT 1;';
		if (! ($stmt = $mysqli->prepare ( $q ))) {
			throw new Exception ( "Prepare failed: (" . $mysqli->errno . ") " . $mysqli->error );
		}
		if (! $stmt->execute ()) {
			throw new Exception ( "Execute failed: (" . $stmt->errno . ") " . $stmt->error );
		}
		$stmt->bind_result ( $id );
		$stmt->fetch ();
		$stmt->close ();
		return ($id) ? $id : 0;
	}
}