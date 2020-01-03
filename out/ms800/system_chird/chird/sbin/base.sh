#! /bin/sh

Usage()
{
	echo "Usage: base.sh get <option> "
	echo "       base.sh set <option> <value>"
	echo "option : "
	echo "      version     -- version of system" 
	echo "      apprun      -- wmpbox run flag"
	echo "      platform    -- platform of the device"
	echo "      hostname    -- hostname of device"
	echo "      login       -- web login"
	echo "      password    -- web password"
	echo "      language    -- web language"
	echo "      id          -- device id"
	echo "      alias       -- device alias"
	echo "      encrypt     -- encrypt string"
	echo "      company     -- company of this device belong to"
	echo "      p2psupport  -- Whether to support the p2p(Just for get) "
	echo "      p2pDID1     -- p2p DID1(Just for get)"
	echo "      exter4g     -- 4G modules or Phone USBNet network status(Just for get)"
	echo "Example: "
	echo "      base.sh get login"
	echo "      base.sh set login admin"
}

if [ $# -lt 2 ] || [ "$1" != "get" -a "$1" != "set" ]; then
	Usage
	exit 1
fi

param="$2"

if [ "$1" = "get" -a $# -eq 2 ]; then
	case $param in
	version)
		echo "`chd_cfg_get system.cfg system_version`";;
	apprun)
		echo "`chd_cfg_get system.cfg system_apprun`";;
	platform)
		echo "`chd_cfg_get system.cfg system_platform`";;
	hostname)
		echo "`chd_cfg_get system.cfg system_hostname`";;
	login)
		echo "`chd_cfg_get system.cfg system_login`";;
	password)
		echo "`chd_cfg_get system.cfg system_password`";;
	language)
		echo "`chd_cfg_get system.cfg system_language`";;
	id)
		echo "`chd_cfg_get system.cfg system_id`";;
	alias)
		echo "`chd_cfg_get system.cfg system_alias`";;
	encrypt)
		echo "`chd_cfg_get system.cfg system_encrypt`";;
	company)
		echo "`chd_cfg_get system.cfg system_company`";;
	p2psupport)
		echo "`chd_cfg_get network.cfg network_p2psupport`";;
	p2pDID1)
		echo "`chd_cfg_get network.cfg network_p2pDID1`";;
	exter4g)
		platform_exter4g_status_get.sh;;
	*)
		echo "Error: $param not found."
		exit 1;;
	esac
elif [ "$1" = "set" -a $# -eq 3 ]; then
	value="$3"

	case $param in
	version)
		chd_cfg_set system.cfg system_version "$value";;
	apprun)
		chd_cfg_set system.cfg system_apprun "$value";;
	platform)
		chd_cfg_set system.cfg system_platform "$value";;
	hostname)
		chd_cfg_set system.cfg system_hostname "$value";;
	login)
		chd_cfg_set system.cfg system_login "$value";;
	password)
		chd_cfg_set system.cfg system_password "$value";;
	language)
		chd_cfg_set system.cfg system_language "$value";;
	id)
		chd_cfg_set system.cfg system_id "$value";;
	alias)
		chd_cfg_set system.cfg system_alias "$value";;
	encrypt)
		chd_cfg_set system.cfg system_encrypt"$value";;
	company)
		chd_cfg_set system.cfg system_company "$value";;
	*)
		echo "Error: $param not found."
		exit 1;;
	esac
else
	Usage
	exit 1
fi
	
exit 0
