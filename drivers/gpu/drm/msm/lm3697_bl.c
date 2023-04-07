/*
 * TI LM3697 Backlight Driver
 *
 * Copyright 2014 Texas Instruments
 *
 * Author: Milo Kim <milo.kim@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pwm.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include "lm3697_bl.h"
#include <soc/oppo/boot_mode.h>
#include <soc/oppo/oppo_project.h>
#include <soc/oppo/device_info.h>

/* Registers */
#define LM3697_REG_OUTPUT_CFG   0x10

#define LM3697_REG_BRT_CFG      0x16

#define LM3697_REG_BOOST_CTL    0x1A
#define LM3697_REG_BOOST        0x06

#define LM3697_REG_PWM_CFG      0x1C

#define LM3697_REG_IMAX_A       0x17
#define LM3697_REG_IMAX_B       0x18

#define LM3697_REG_BRT_A_LSB        0x20
#define LM3697_REG_BRT_A_MSB        0x21
#define LM3697_REG_BRT_B_LSB        0x22
#define LM3697_REG_BRT_B_MSB        0x23
#define LM3697_BRT_LSB_MASK     (BIT(0) | BIT(1) | BIT(2))
#define LM3697_BRT_MSB_SHIFT        3

#define LM3697_REG_ENABLE       0x24

/* Other definitions */
#define BACKLIGHT_EXPONENTIAL 1
#define LM3697_PWM_ID           1
#define LM3697_MAX_REGISTERS        0xB4
#define LM3697_MAX_STRINGS      3
#define LM3697_MAX_BRIGHTNESS       2047
#define LM3697_IMAX_OFFSET      6
#define LM3697_DEFAULT_NAME     "lcd-backlight"
#define LM3697_DEFAULT_PWM      "lm3697-backlight"

//add for lm3697 bl change to exponential mode
#define LM3697_EXPONENTIAL      1
#define LM3697_LINEAR           0
#define LM3697_LEVEL_2048_SUPPROT    1

#ifdef VENDOR_EDIT
#define REG_PWM     0x1C
#define CABC_DISABLE_LEVEL  160
#define LM3697_MIN_BRIGHTNESS 6
#define FILTER_STR 0x50
#define RUN_RAMP 0x08
#define REG_REVISION 0x1F
static bool pwm_flag = false;
//static int backlight_level = 2047;

extern int cabc_mode;
int set_backlight_pwm(int state);
#endif /*VENDOR_EDIT*/


enum lm3697_bl_ctrl_mode {
    BL_REGISTER_BASED,
    BL_PWM_BASED,
};

/*
 * struct lm3697_bl_chip
 * @dev: Parent device structure
 * @regmap: Used for I2C register access
 * @pdata: LM3697 platform data
 */
struct lm3697_bl_chip {
    struct device *dev;
    struct lm3697_platform_data *pdata;
    struct regmap *regmap;

};

/*
 * struct lm3697_bl
 * @bank_id: Control bank ID. BANK A or BANK A and B
 * @bl_dev: Backlight device structure
 * @chip: LM3697 backlight chip structure for low level access
 * @bl_pdata: LM3697 backlight platform data
 * @mode: Backlight control mode
 * @pwm: PWM device structure. Only valid in PWM control mode
 * @pwm_name: PWM device name
 */
struct lm3697_bl {
    int bank_id;
    struct backlight_device *bl_dev;
    struct lm3697_bl_chip *chip;
    struct lm3697_backlight_platform_data *bl_pdata;
    enum lm3697_bl_ctrl_mode mode;
    struct pwm_device *pwm;
    char pwm_name[20];
};

static struct lm3697_bl_chip *lm3697_pchip;

int is_ktd3136 = 0;

//add for lm3697 bl change to exponential mode
//add for silence and sau mode bl isn't black
static int backlight_buf[] = {
    0, 388, 388, 412, 430, 446, 459, 471, 483, 493, 503, 511, 518, 524, 529, 533, 536, 540, 542, 543, \
    546, 548, 551, 553, 554, 557, 559, 561, 564, 565, 567, 570, 572, 573, 576, 578, 579, 582, 584, 585, \
    588, 590, 591, 594, 596, 597, 599, 602, 603, 605, 607, 609, 611, 613, 615, 616, 619, 620, 622, 623, \
    626, 628, 629, 632, 633, 635, 636, 639, 640, 642, 644, 646, 647, 648, 651, 652, 654, 656, 658, 659, \
    662, 663, 664, 666, 667, 670, 671, 672, 675, 676, 678, 679, 681, 683, 684, 685, 688, 689, 690, 693, \
    694, 695, 697, 698, 700, 702, 703, 704, 707, 708, 709, 712, 713, 714, 715, 718, 719, 720, 721, 724, \
    725, 726, 727, 730, 731, 732, 733, 735, 737, 738, 739, 741, 743, 744, 745, 746, 749, 750, 751, 752, \
    753, 756, 757, 758, 759, 761, 762, 764, 765, 766, 768, 769, 770, 771, 774, 775, 776, 777, 778, 780, \
    781, 783, 784, 786, 787, 788, 789, 790, 792, 793, 795, 796, 798, 799, 800, 801, 802, 803, 805, 806, \
    807, 809, 811, 812, 813, 814, 815, 817, 818, 819, 820, 821, 823, 824, 825, 826, 827, 829, 830, 831, \
    832, 833, 836, 837, 838, 839, 840, 842, 843, 844, 845, 846, 848, 849, 850, 851, 852, 854, 855, 856, \
    857, 858, 860, 861, 862, 863, 864, 864, 866, 867, 868, 869, 870, 871, 873, 874, 875, 876, 877, 879, \
    880, 881, 882, 883, 885, 886, 887, 888, 889, 889, 891, 892, 893, 894, 895, 897, 898, 899, 900, 901, \
    902, 904, 904, 905, 906, 907, 908, 910, 911, 912, 913, 914, 916, 916, 917, 918, 919, 920, 922, 923, \
    924, 925, 925, 926, 928, 929, 930, 931, 932, 934, 934, 935, 936, 937, 938, 939, 941, 942, 942, 943, \
    944, 945, 947, 948, 949, 949, 950, 951, 953, 954, 955, 956, 956, 957, 959, 960, 961, 962, 962, 963, \
    965, 966, 967, 968, 968, 969, 970, 972, 973, 974, 974, 975, 976, 978, 979, 979, 980, 981, 982, 984, \
    985, 985, 986, 987, 988, 990, 990, 991, 992, 993, 994, 994, 996, 997, 998, 999, 999, 1000, 1002, 1003, \
    1004, 1004, 1005, 1006, 1007, 1007, 1009, 1010, 1011, 1012, 1012, 1013, 1015, 1016, 1016, 1017, 1018, 1019, 1021, 1021, \
    1022, 1023, 1024, 1024, 1025, 1027, 1028, 1028, 1029, 1030, 1031, 1033, 1033, 1034, 1035, 1036, 1036, 1037, 1038, 1040, \
    1040, 1041, 1042, 1043, 1043, 1044, 1046, 1047, 1047, 1048, 1049, 1050, 1050, 1052, 1053, 1054, 1054, 1055, 1056, 1056, \
    1058, 1059, 1060, 1060, 1061, 1062, 1064, 1064, 1065, 1066, 1067, 1067, 1068, 1069, 1069, 1071, 1072, 1073, 1073, 1074, \
    1075, 1077, 1077, 1078, 1079, 1079, 1080, 1081, 1083, 1083, 1084, 1085, 1085, 1086, 1087, 1089, 1089, 1090, 1091, 1091, \
    1092, 1093, 1093, 1095, 1096, 1097, 1097, 1098, 1099, 1099, 1101, 1102, 1102, 1103, 1104, 1105, 1105, 1106, 1108, 1108, \
    1109, 1110, 1110, 1111, 1112, 1112, 1114, 1115, 1116, 1116, 1117, 1118, 1118, 1120, 1121, 1121, 1122, 1123, 1123, 1124, \
    1126, 1126, 1127, 1128, 1128, 1129, 1130, 1130, 1132, 1133, 1134, 1134, 1135, 1136, 1136, 1137, 1139, 1139, 1140, 1141, \
    1141, 1142, 1143, 1143, 1145, 1146, 1146, 1147, 1148, 1148, 1149, 1151, 1151, 1152, 1152, 1153, 1154, 1154, 1155, 1157, \
    1157, 1158, 1159, 1159, 1160, 1161, 1161, 1163, 1164, 1164, 1165, 1166, 1166, 1167, 1169, 1169, 1170, 1171, 1171, 1172, \
    1172, 1173, 1174, 1174, 1176, 1177, 1177, 1178, 1179, 1179, 1180, 1182, 1182, 1183, 1183, 1184, 1185, 1185, 1186, 1188, \
    1188, 1189, 1190, 1190, 1191, 1191, 1192, 1194, 1194, 1195, 1196, 1196, 1197, 1198, 1198, 1200, 1200, 1201, 1202, 1202, \
    1203, 1204, 1204, 1205, 1205, 1207, 1208, 1208, 1209, 1210, 1210, 1211, 1211, 1213, 1214, 1214, 1215, 1216, 1216, 1217, \
    1217, 1219, 1220, 1220, 1221, 1221, 1222, 1223, 1223, 1225, 1226, 1226, 1227, 1227, 1228, 1229, 1229, 1231, 1231, 1232, \
    1233, 1233, 1234, 1234, 1235, 1237, 1237, 1238, 1239, 1239, 1240, 1240, 1241, 1242, 1242, 1244, 1244, 1245, 1246, 1246, \
    1247, 1247, 1248, 1250, 1250, 1251, 1251, 1252, 1253, 1253, 1254, 1254, 1256, 1257, 1257, 1258, 1258, 1259, 1260, 1260, \
    1262, 1262, 1263, 1264, 1264, 1265, 1265, 1266, 1268, 1268, 1269, 1269, 1270, 1270, 1271, 1272, 1272, 1273, 1273, 1275, \
    1276, 1276, 1277, 1277, 1278, 1279, 1279, 1281, 1281, 1282, 1282, 1283, 1284, 1284, 1285, 1285, 1287, 1288, 1288, 1289, \
    1289, 1290, 1290, 1291, 1293, 1293, 1294, 1294, 1295, 1295, 1296, 1297, 1297, 1299, 1299, 1300, 1301, 1301, 1302, 1302, \
    1303, 1303, 1305, 1306, 1306, 1307, 1307, 1308, 1308, 1309, 1310, 1310, 1312, 1312, 1313, 1313, 1314, 1315, 1315, 1316, \
    1316, 1318, 1318, 1319, 1320, 1320, 1321, 1321, 1322, 1322, 1324, 1325, 1325, 1326, 1326, 1327, 1327, 1328, 1328, 1330, \
    1331, 1331, 1332, 1332, 1333, 1333, 1334, 1336, 1336, 1337, 1337, 1338, 1338, 1339, 1339, 1340, 1341, 1341, 1343, 1343, \
    1344, 1344, 1345, 1345, 1346, 1347, 1347, 1349, 1349, 1350, 1350, 1351, 1351, 1352, 1353, 1353, 1355, 1355, 1356, 1356, \
    1357, 1357, 1358, 1359, 1359, 1361, 1361, 1362, 1362, 1363, 1363, 1364, 1365, 1365, 1367, 1367, 1368, 1368, 1369, 1369, \
    1370, 1370, 1371, 1372, 1372, 1374, 1374, 1375, 1375, 1376, 1376, 1377, 1377, 1378, 1380, 1380, 1381, 1381, 1382, 1382, \
    1383, 1383, 1384, 1384, 1386, 1387, 1387, 1388, 1388, 1389, 1389, 1390, 1390, 1392, 1392, 1393, 1393, 1394, 1395, 1395, \
    1396, 1396, 1398, 1398, 1399, 1399, 1400, 1400, 1401, 1401, 1402, 1404, 1404, 1405, 1405, 1406, 1406, 1407, 1407, 1408, \
    1408, 1409, 1409, 1411, 1412, 1412, 1413, 1413, 1414, 1414, 1415, 1415, 1417, 1417, 1418, 1418, 1419, 1419, 1420, 1420, \
    1421, 1423, 1423, 1424, 1424, 1425, 1425, 1426, 1426, 1427, 1427, 1429, 1429, 1430, 1430, 1431, 1431, 1432, 1433, 1433, \
    1435, 1435, 1436, 1436, 1437, 1437, 1438, 1438, 1439, 1439, 1440, 1440, 1442, 1442, 1443, 1443, 1444, 1444, 1445, 1446, \
    1446, 1448, 1448, 1449, 1449, 1450, 1450, 1451, 1451, 1452, 1452, 1454, 1454, 1455, 1455, 1456, 1456, 1457, 1457, 1458, \
    1458, 1460, 1461, 1461, 1462, 1462, 1463, 1463, 1464, 1464, 1466, 1466, 1467, 1467, 1468, 1468, 1469, 1469, 1470, 1470, \
    1472, 1472, 1473, 1473, 1474, 1474, 1475, 1475, 1476, 1476, 1477, 1477, 1479, 1480, 1480, 1481, 1481, 1482, 1482, 1483, \
    1483, 1485, 1485, 1486, 1486, 1487, 1487, 1488, 1488, 1489, 1489, 1491, 1491, 1492, 1492, 1493, 1493, 1494, 1494, 1495, \
    1495, 1497, 1497, 1498, 1498, 1499, 1499, 1500, 1500, 1501, 1501, 1503, 1503, 1504, 1504, 1505, 1505, 1506, 1506, 1507, \
    1507, 1508, 1508, 1510, 1510, 1511, 1511, 1512, 1513, 1513, 1514, 1514, 1516, 1516, 1517, 1517, 1518, 1518, 1519, 1519, \
    1520, 1520, 1522, 1522, 1523, 1523, 1524, 1524, 1525, 1525, 1526, 1526, 1528, 1528, 1529, 1529, 1530, 1530, 1531, 1531, \
    1532, 1532, 1534, 1534, 1535, 1535, 1536, 1536, 1537, 1537, 1538, 1538, 1540, 1540, 1541, 1541, 1542, 1542, 1543, 1543, \
    1544, 1544, 1545, 1545, 1547, 1547, 1548, 1548, 1549, 1549, 1550, 1550, 1551, 1551, 1553, 1553, 1554, 1554, 1554, 1555, \
    1555, 1556, 1556, 1557, 1557, 1559, 1559, 1560, 1560, 1561, 1561, 1562, 1562, 1563, 1563, 1565, 1565, 1566, 1566, 1567, \
    1567, 1568, 1568, 1569, 1569, 1571, 1571, 1572, 1572, 1573, 1573, 1574, 1574, 1575, 1575, 1576, 1576, 1578, 1578, 1579, \
    1579, 1580, 1580, 1581, 1581, 1582, 1582, 1584, 1584, 1585, 1585, 1586, 1586, 1587, 1587, 1588, 1588, 1590, 1590, 1590, \
    1591, 1591, 1592, 1592, 1593, 1593, 1594, 1594, 1596, 1596, 1597, 1597, 1598, 1598, 1599, 1599, 1600, 1600, 1602, 1602, \
    1603, 1603, 1604, 1604, 1605, 1605, 1606, 1606, 1608, 1608, 1609, 1609, 1610, 1610, 1610, 1611, 1611, 1612, 1612, 1613, \
    1613, 1615, 1615, 1616, 1616, 1617, 1617, 1618, 1618, 1619, 1619, 1621, 1621, 1622, 1622, 1623, 1623, 1624, 1624, 1625, \
    1625, 1627, 1627, 1627, 1628, 1628, 1629, 1629, 1630, 1630, 1631, 1631, 1633, 1633, 1634, 1634, 1635, 1635, 1636, 1636, \
    1637, 1637, 1639, 1639, 1640, 1640, 1641, 1641, 1641, 1642, 1642, 1643, 1643, 1644, 1644, 1646, 1646, 1647, 1647, 1648, \
    1648, 1649, 1649, 1650, 1650, 1652, 1652, 1653, 1653, 1654, 1654, 1654, 1655, 1655, 1656, 1656, 1658, 1658, 1659, 1659, \
    1660, 1660, 1661, 1661, 1662, 1662, 1664, 1664, 1665, 1665, 1666, 1666, 1666, 1667, 1667, 1668, 1668, 1670, 1670, 1671, \
    1671, 1672, 1672, 1673, 1673, 1674, 1674, 1676, 1676, 1677, 1677, 1677, 1678, 1678, 1679, 1679, 1680, 1680, 1681, 1681, \
    1683, 1683, 1684, 1684, 1685, 1685, 1686, 1686, 1686, 1687, 1687, 1689, 1689, 1690, 1690, 1691, 1691, 1692, 1692, 1693, \
    1693, 1695, 1695, 1696, 1696, 1697, 1697, 1697, 1698, 1698, 1699, 1699, 1701, 1701, 1702, 1702, 1703, 1703, 1704, 1704, \
    1705, 1705, 1705, 1707, 1707, 1708, 1708, 1709, 1709, 1710, 1710, 1711, 1711, 1712, 1712, 1714, 1714, 1715, 1715, 1715, \
    1716, 1716, 1717, 1717, 1718, 1718, 1720, 1720, 1721, 1721, 1722, 1722, 1723, 1723, 1723, 1724, 1724, 1726, 1726, 1727, \
    1727, 1728, 1728, 1729, 1729, 1730, 1730, 1732, 1732, 1732, 1733, 1733, 1734, 1734, 1735, 1735, 1736, 1736, 1738, 1738, \
    1739, 1739, 1739, 1740, 1740, 1741, 1741, 1742, 1742, 1743, 1743, 1745, 1745, 1746, 1746, 1747, 1747, 1747, 1748, 1748, \
    1749, 1749, 1751, 1751, 1752, 1752, 1753, 1753, 1754, 1754, 1754, 1755, 1755, 1757, 1757, 1758, 1758, 1759, 1759, 1760, \
    1760, 1761, 1761, 1761, 1763, 1763, 1764, 1764, 1765, 1765, 1766, 1766, 1767, 1767, 1769, 1769, 1769, 1770, 1770, 1771, \
    1771, 1772, 1772, 1773, 1773, 1775, 1775, 1776, 1776, 1776, 1777, 1777, 1778, 1778, 1779, 1779, 1780, 1780, 1782, 1782, \
    1783, 1783, 1783, 1784, 1784, 1785, 1785, 1786, 1786, 1788, 1788, 1789, 1789, 1789, 1790, 1790, 1791, 1791, 1792, 1792, \
    1794, 1794, 1795, 1795, 1796, 1796, 1796, 1797, 1797, 1798, 1798, 1800, 1800, 1801, 1801, 1802, 1802, 1802, 1803, 1803, \
    1804, 1804, 1806, 1806, 1807, 1807, 1808, 1808, 1809, 1809, 1809, 1810, 1810, 1811, 1811, 1813, 1813, 1814, 1814, 1815, \
    1815, 1815, 1816, 1816, 1817, 1817, 1819, 1819, 1820, 1820, 1821, 1821, 1821, 1822, 1822, 1823, 1823, 1825, 1825, 1826, \
    1826, 1827, 1827, 1827, 1828, 1828, 1829, 1829, 1831, 1831, 1832, 1832, 1833, 1833, 1833, 1834, 1834, 1835, 1835, 1837, \
    1837, 1838, 1838, 1839, 1839, 1839, 1840, 1840, 1841, 1841, 1843, 1843, 1844, 1844, 1845, 1845, 1845, 1846, 1846, 1847, \
    1847, 1848, 1848, 1850, 1850, 1851, 1851, 1851, 1852, 1852, 1853, 1853, 1854, 1854, 1856, 1856, 1857, 1857, 1857, 1858, \
    1858, 1859, 1859, 1860, 1860, 1862, 1862, 1862, 1863, 1863, 1864, 1864, 1865, 1865, 1866, 1866, 1868, 1868, 1868, 1869, \
    1869, 1870, 1870, 1871, 1871, 1872, 1872, 1874, 1874, 1874, 1875, 1875, 1876, 1876, 1877, 1877, 1878, 1878, 1878, 1879, \
    1879, 1881, 1881, 1882, 1882, 1883, 1883, 1884, 1884, 1884, 1885, 1885, 1887, 1887, 1888, 1888, 1889, 1889, 1889, 1890, \
    1890, 1891, 1891, 1893, 1893, 1894, 1894, 1894, 1895, 1895, 1896, 1896, 1897, 1897, 1899, 1899, 1900, 1900, 1900, 1901, \
    1901, 1902, 1902, 1903, 1903, 1905, 1905, 1905, 1906, 1906, 1907, 1907, 1908, 1908, 1909, 1909, 1911, 1911, 1911, 1912, \
    1912, 1913, 1913, 1914, 1914, 1915, 1915, 1915, 1916, 1916, 1918, 1918, 1919, 1919, 1920, 1920, 1920, 1921, 1921, 1922, \
    1922, 1924, 1924, 1925, 1925, 1925, 1926, 1926, 1927, 1927, 1928, 1928, 1930, 1930, 1930, 1931, 1931, 1932, 1932, 1933, \
    1933, 1934, 1934, 1936, 1936, 1936, 1937, 1937, 1938, 1938, 1939, 1939, 1940, 1940, 1940, 1942, 1942, 1943, 1943, 1944, \
    1944, 1945, 1945, 1945, 1946, 1946, 1947, 1947, 1949, 1949, 1950, 1950, 1950, 1951, 1951, 1952, 1952, 1953, 1953, 1955, \
    1955, 1955, 1956, 1956, 1957, 1957, 1958, 1958, 1959, 1959, 1959, 1961, 1961, 1962, 1962, 1963, 1963, 1964, 1964, 1964, \
    1965, 1965, 1967, 1967, 1968, 1968, 1969, 1969, 1969, 1970, 1970, 1971, 1971, 1973, 1973, 1974, 1974, 1974, 1975, 1975, \
    1976, 1976, 1977, 1977, 1979, 1979, 1979, 1980, 1980, 1981, 1981, 1982, 1982, 1982, 1983, 1983, 1984, 1984, 1986, 1986, \
    1987, 1987, 1987, 1988, 1988, 1989, 1989, 1990, 1990, 1992, 1992, 1992, 1993, 1993, 1994, 1994, 1995, 1995, 1996, 1996, \
    1996, 1998, 1998, 1999, 1999, 2000, 2000, 2001, 2001, 2001, 2002, 2002, 2004, 2004, 2005, 2005, 2006, 2006, 2006, 2007, \
    2007, 2008, 2008, 2009, 2009, 2009, 2010, 2010, 2011, 2011, 2011, 2012, 2012, 2013, 2013, 2013, 2014, 2014, 2015, 2015, \
    2015, 2016, 2016, 2017, 2017, 2017, 2018, 2018, 2019, 2019, 2019, 2020, 2020, 2021, 2021, 2021, 2022, 2022, 2023, 2023, \
    2024, 2024, 2025, 2025, 2026, 2026, 2027, 2027, 2028, 2028, 2029, 2029, 2030, 2030, 2031, 2031, 2032, 2032, 2033, 2033, \
    2034, 2034, 2035, 2035, 2036, 2036, 2037, 2037, 2038, 2038, 2039, 2039, 2040, 2040, 2041, 2041, 2042, 2042, 2043, 2043, \
    2044, 2044, 2045, 2045, 2046, 2046, 2047, 2047
};

int lm3697_lcd_backlight_set_level(unsigned int bl_level)
{
	struct lm3697_bl_chip *pchip = lm3697_pchip;
	unsigned int BL_MSB =0;
	unsigned int BL_LSB =0;
	int ret = 0;

	if (!pchip || !lm3697_pchip) {
		dev_err(pchip->dev, "lm3697_lcd_backlight_set_level pchip is null\n");
		return 0;
	}

	if (!pchip->regmap || !lm3697_pchip->regmap) {
		pr_err("%s  pchip->regmap is NULL.\n", __func__);
		return 0;
	}

	if (is_ktd3136 == 1) {
		if(BACKLIGHT_EXPONENTIAL){
			BL_MSB = (backlight_buf[bl_level] >>3) & 0xFF;
			BL_LSB = backlight_buf[bl_level] & 0x07;
		}else{
			BL_MSB = (bl_level >>3) & 0xFF;
			BL_LSB = bl_level & 0x07;
		}

        pr_err("[oppo_test] in ktd3136 bl_level=%d, LSB=0x%x, MSB=0x%x\n", bl_level,BL_LSB,BL_MSB);

		/* brightness 0 means disable */
		if (bl_level==0) {
			ret = regmap_write(pchip->regmap, 0x04, BL_LSB);
			if (ret < 0)
				goto out;
			ret = regmap_write(pchip->regmap, 0x05, BL_MSB);
			if (ret < 0)
				goto out;
		} else {
			ret = regmap_write(pchip->regmap, 0x04, BL_LSB);
			if (ret < 0)
				goto out;
			ret = regmap_write(pchip->regmap, 0x05, BL_MSB);
			if (ret < 0)
				goto out;

			ret = regmap_write(pchip->regmap, 0x02, 0x99);
			if (ret < 0)
				goto out;
		}
	} else {
        //do nothing
	}

	return ret;
out:
	pr_err("%s  set lcd backlight failed.\n", __func__);
	return ret;
}

EXPORT_SYMBOL(lm3697_lcd_backlight_set_level);

static int lm3697_dt(struct device *dev, struct lm3697_platform_data *pdata)
{
    struct device_node *np = dev->of_node;

    pdata->en_gpio = of_get_named_gpio(np, "ti,bl-enable-gpio", 0);

    pr_err("%s bl_en_gpio=%d\n", __func__, pdata->en_gpio);

    if (!gpio_is_valid(pdata->en_gpio))
            pr_err("%s:%d, Backlight_en gpio not specified\n", __func__, __LINE__);

    return 0;
}

//#ifdef VENDOR_EDIT
//add for lm3697 reg init
int lm3697_reg_init(void){
	struct lm3697_bl_chip *pchip = lm3697_pchip;
	int ret = 0;

    pr_err("lm3697_reg_init\n");
	if (is_ktd3136 == 1) {
		ret = regmap_write(pchip->regmap, 0x02, 0x98);  //OVP 32V, Freq 500kh
		if (ret < 0)
			goto out;

		ret = regmap_write(pchip->regmap, 0x03, 0x28);    //Exponential Mapping Mode
		if (ret < 0)
			goto out;

		ret = regmap_write(pchip->regmap, 0x06, 0x23);  //Linear Mapping Mode
		if (ret < 0)
			goto out;

		ret = regmap_write(pchip->regmap, 0x07, 0x00);  //Bank A Full-scale current (17.8mA)
		if (ret < 0)
			goto out;

		ret = regmap_write(pchip->regmap, 0x08, 0x10);    //Bank B Full-scale current (17.8mA)
		if (ret < 0)
			goto out;
	} else {
        //do nothing
	}

	return ret;
out:
	dev_err(pchip->dev, "i2c failed to access register\n");
	return ret;
}

void lm3697_bl_enable(int enable){

    struct lm3697_bl_chip *pchip = lm3697_pchip;

    if(!pchip || !lm3697_pchip){
        pr_err("%s  lm3697_bl_enable pdata is null.\n", __func__);
        return;
    }
    if (!pchip->regmap || !lm3697_pchip->regmap || !lm3697_pchip->pdata) {
        pr_err("%s  pchip->regmap is NULL.\n", __func__);
        return;
    }

    pr_info("mdss %s  = %d\n", __func__, enable);

    if(enable) {
        if (gpio_is_valid(pchip->pdata->en_gpio)){
            gpio_set_value((pchip->pdata->en_gpio), 1);
        }else{
            pr_err("%s: enable failed", __func__);
        }
    } else {
        if (gpio_is_valid(pchip->pdata->en_gpio)){
            gpio_set_value((pchip->pdata->en_gpio), 0);
        }else{
            pr_err("%s: disable failed", __func__);
        }
    }
}
EXPORT_SYMBOL(lm3697_bl_enable);

int set_backlight_pwm(int state)
{
    int rc = 0;
    pr_err("%s: state = %d pwm_flag = %d\n", __func__, state, pwm_flag);
    return rc;
    //Add for solve coverity error.
    /*if(!lm3697_pchip->regmap) {
        pr_err("%s  lm3697_pchip->regmap is NULL.\n", __func__);
        return 0;
    }
    pr_err("%s: state = %d pwm_flag = %d\n", __func__, state, pwm_flag);
    return rc;
    */
}
EXPORT_SYMBOL(set_backlight_pwm);
//#endif /*VENDOR_EDIT*/

static int lm3697_chip_init(struct lm3697_bl_chip *pchip){
	int ret = 0;
	unsigned int revision;
    int i = 0;

    while(i < 3)
    {
        ret = regmap_read(pchip->regmap, 0x00, &revision);
        if(ret == 0)
            break;
        i++;
    }

    if((ret < 0) && (i==3)){
        is_ktd3136 = -1;
        pr_err("not a i2c backlight ic\n");
        register_device_proc("backlight", "aw9962", "aw9962");
        goto out;
    }

	if (revision == 0x18) {
		is_ktd3136 = 1;
        register_device_proc("backlight", "ktd3136", "ktd3136");
	}

	pr_err("0x00 revision is 0x%x\n", revision);

	if (is_ktd3136 == 1) {
		/*
		* add for boot mode
		*/
		if (MSM_BOOT_MODE__SILENCE == get_boot_mode()
			|| MSM_BOOT_MODE__SAU == get_boot_mode()) {
			ret = regmap_write(pchip->regmap, 0x02, 0x98);  //OVP 32V, Freq 500kh
			if (ret < 0)
				goto out;
		} else {
			ret = regmap_write(pchip->regmap, 0x02, 0x99);  //OVP 32V, Freq 500kh
			if (ret < 0)
				goto out;
		}

		ret = regmap_write(pchip->regmap, 0x03, 0x28);    //Exponential Mapping Mode
		if (ret < 0)
			goto out;

		ret = regmap_write(pchip->regmap, 0x06, 0x23);  //Linear Mapping Mode
		if (ret < 0)
			goto out;

		ret = regmap_write(pchip->regmap, 0x07, 0x00);  //Bank A Full-scale current (17.8mA)
		if (ret < 0)
			goto out;

		ret = regmap_write(pchip->regmap, 0x08, 0x10);    //Bank B Full-scale current (17.8mA)
		if (ret < 0)
			goto out;
	} else {
        pr_err("other i2c backlight ic\n");
        register_device_proc("backlight", "unknown", "unknown");
        //do nothing.
	}

	return ret;
out:
    dev_err(pchip->dev, "i2c failed to access register\n");
    return ret;
}

static struct regmap_config lm3697_regmap = {
    .reg_bits = 8,
    .val_bits = 8,
    .max_register = LM3697_MAX_REGISTERS,
};

static int lm3697_bl_probe(struct i2c_client *client,
               const struct i2c_device_id *id)
{
    struct lm3697_platform_data *pdata = client->dev.platform_data;
    struct lm3697_bl_chip *pchip;
    unsigned int revision;
    static char *temp;
    int ret = 0;

    pr_err("%s Enter\n", __func__);

    if (client->dev.of_node) {
        pdata = devm_kzalloc(&client->dev,
            sizeof(struct lm3697_platform_data), GFP_KERNEL);
        if (!pdata) {
            dev_err(&client->dev, "Failed to allocate memory\n");
            return -ENOMEM;
        }

        ret = lm3697_dt(&client->dev, pdata);
        if (ret)
            return ret;
    } else
        pdata = client->dev.platform_data;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_err(&client->dev, "fail : i2c functionality check...\n");
        return -EOPNOTSUPP;
    }

    if (pdata == NULL) {
        dev_err(&client->dev, "fail : no platform data.\n");
        return -ENODATA;
    }

    pchip = devm_kzalloc(&client->dev, sizeof(struct lm3697_bl_chip),
                 GFP_KERNEL);
    if (!pchip)
        return -ENOMEM;
    lm3697_pchip = pchip;


    pchip->pdata = pdata;
    pchip->dev = &client->dev;

    ret = gpio_request(pdata->en_gpio, "backlight_enable");
    if (ret) {
        pr_err("request enable gpio failed, ret=%d\n", ret);
    }
    pr_err("%s bl_en_gpio=%d\n", __func__, pdata->en_gpio);

    if (gpio_is_valid(pdata->en_gpio)){
        gpio_set_value((pdata->en_gpio), 1);
        gpio_direction_output((pdata->en_gpio), 1);
    }

    pchip->regmap = devm_regmap_init_i2c(client, &lm3697_regmap);
    if (IS_ERR(pchip->regmap)) {
        ret = PTR_ERR(pchip->regmap);
        dev_err(&client->dev, "fail : allocate register map: %d\n",
            ret);
        return ret;
    }

    i2c_set_clientdata(client, pchip);

    /* chip initialize */
    ret = lm3697_chip_init(pchip);
    if (ret < 0) {
        dev_err(&client->dev, "fail : init chip\n");
        goto error_enable;
    }

    regmap_read(pchip->regmap,0x00,&revision);
    if (revision == 0x18) {
        temp = "18";
    }else if (revision == 0x00) {
        temp = "00";
    }else {
        temp = "unknown";
    }

    pr_err("%s :revision = %s\n", __func__, temp);

    //register_device_proc("backlight", temp, "LM3697");
    pr_info("%s : probe done\n", __func__);

    return 0;


error_enable:
    /* chip->pdata and chip->pdata->bl_pdata
     * are allocated in lm3697_bl_parse_dt() by devm_kzalloc()
     */

    devm_kfree(&client->dev, pchip->pdata);
    devm_kfree(&client->dev, pchip);
    pr_err("%s : probe failed\n", __func__);
    return ret;
}

static int lm3697_bl_remove(struct i2c_client *client){
    struct lm3697_bl_chip *pchip = i2c_get_clientdata(client);
    int ret = 0;

    pr_err("%s :  failed\n", __func__);

    if(is_ktd3136 > 0){
        ret = regmap_write(pchip->regmap, LM3697_REG_BRT_A_LSB, 0);
        if (ret < 0)
            dev_err(pchip->dev, "i2c failed to access register\n");

        ret = regmap_write(pchip->regmap, LM3697_REG_BRT_A_MSB, 0);
        if (ret < 0)
            dev_err(pchip->dev, "i2c failed to access register\n");
    }

    if (gpio_is_valid(pchip->pdata->en_gpio)){
        gpio_set_value(pchip->pdata->en_gpio, 0);
        gpio_free(pchip->pdata->en_gpio);
    }

    return 0;
}

static const struct i2c_device_id lm3697_bl_ids[] = {
    { "lm3697", 0 },
    { }
};


static struct i2c_driver lm3697_i2c_driver = {
    .probe = lm3697_bl_probe,
    .remove = lm3697_bl_remove,
    .driver = {
        .name = "lm3697",
        .owner = THIS_MODULE,
    },
    .id_table = lm3697_bl_ids,
};

module_i2c_driver(lm3697_i2c_driver);

MODULE_DESCRIPTION("TI LM3697 Backlight Driver");
MODULE_AUTHOR("Milo Kim");
MODULE_LICENSE("GPL");
