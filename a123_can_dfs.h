/*
  mcp_can_dfs.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  Contributor: Cory J. Fowler
  2014-1-16
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/
#ifndef _A123DFS_H_
#define _A123DFS_H_

/*
 *   BCM_CMD
 */

#define BCM_CMD_ID          0x50
#define BCM_REQUEST_TYPE_SB 0
#define BCM_REQUEST_TYPE_L  2
#define BCM_LEAKAGE_CMD_SB  2
#define BCM_LEAKAGE_CMD_L   2
#define BCM_REQUEST_ID_SB   4
#define BCM_REQUEST_ID_L    4
#define BCM_V_BAL_TARGET_SB 19
#define BCM_V_BAL_TARGET_L  13

/*
** MOD_STD
*/
#define MOD_STD             0x200


/*
** Data Offsets
*/
#define MOD_THERM_OFFSET -40
#define MOD_V_OFFSET 1


#endif
