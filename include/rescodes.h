const struct response responses[] = {
	{200, NULL, NULL},
	{201, NULL, NULL},
	{202, NULL, NULL},
	{203, NULL, NULL},
	{204, NULL, NULL},
	{205, NULL, NULL},
	{206, NULL, NULL},
	{207, NULL, NULL}, /* unsupported */
	{208, NULL, NULL},
	{209, NULL, NULL}, /* unsupported */
	{210, NULL, NULL}, /* unsupported */
	{211, NULL, NULL},
	{212, NULL, NULL},
	{213, NULL, NULL},
	{214, NULL, NULL},
	{215, NULL, NULL},
	{216, NULL, NULL},
	{217, NULL, NULL}, /* unsupported */
	{218, NULL, NULL},
	{219, NULL, NULL},
	{220, NULL, NULL}, /* unsupported */
	{221, NULL, NULL}, /* unsupported */
	{222, NULL, NULL}, /* unsupported */
	{223, NULL, NULL}, /* unsupported */
	{224, NULL, NULL}, /* unsupported */
	{225, NULL, NULL}, /* unsupported */
	{226, NULL, NULL}, /* unsupported */
	{227, NULL, NULL}, /* unsupported */
	{228, NULL, NULL}, /* unsupported */
	{229, NULL, NULL}, /* unsupported */
	{230, NULL, NULL}, /* unsupported */
	{221, NULL, NULL},
	{232, NULL, NULL}, /* unsupported */
	{233, NULL, NULL}, /* unsupported */
	{234, NULL, NULL}, /* unsupported */
	{235, NULL, NULL}, /* unsupported */
	{236, NULL, NULL}, /* unsupported */
	{237, NULL, NULL}, /* unsupported */
	{238, NULL, NULL}, /* unsupported */
	{239, NULL, NULL}, /* unsupported */
	{240, NULL, NULL}, /* unsupported */
	{241, NULL, NULL},
	{242, NULL, NULL},
	{243, NULL, NULL},
	{244, NULL, NULL},
	{245, NULL, NULL}, /* unsupported */
	{246, NULL, NULL}, /* unsupported */
	{247, NULL, NULL}, /* unsupported */
	{248, NULL, NULL}, /* unsupported */
	{249, NULL, NULL}, /* unsupported */
	{250, NULL, NULL}, /* unsupported */
	{251, NULL, NULL},
	{252, NULL, NULL},
	{253, NULL, NULL},
	{254, NULL, NULL},
	{255, NULL, NULL},
	{256, NULL, NULL},
	{257, NULL, NULL},
	{258, NULL, NULL},
	{259, NULL, NULL},
	{260, NULL, NULL}, /* unsupported */
	{261, NULL, NULL},
	{262, NULL, NULL}, /* unsupported */
	{263, NULL, NULL}, /* unsupported */
	{264, NULL, NULL}, /* unsupported */
	{265, NULL, NULL}, /* unsupported */
	{266, NULL, NULL}, /* unsupported */
	{267, NULL, NULL}, /* unsupported */
	{268, NULL, NULL}, /* unsupported */
	{269, NULL, NULL}, /* unsupported */
	{270, NULL, NULL}, /* unsupported */
	{271, NULL, NULL}, /* unsupported */
	{272, NULL, NULL}, /* unsupported */
	{273, NULL, NULL}, /* unsupported */
	{274, NULL, NULL}, /* unsupported */
	{275, NULL, NULL}, /* unsupported */
	{276, NULL, NULL}, /* unsupported */
	{277, NULL, NULL}, /* unsupported */
	{278, NULL, NULL}, /* unsupported */
	{279, NULL, NULL}, /* unsupported */
	{280, NULL, NULL}, /* unsupported */
	{281, NULL, NULL}, /* unsupported */
	{282, NULL, NULL}, /* unsupported */
	{283, NULL, NULL}, /* unsupported */
	{284, NULL, NULL}, /* unsupported */
	{285, NULL, NULL}, /* unsupported */
	{286, NULL, NULL}, /* unsupported */
	{287, NULL, NULL}, /* unsupported */
	{288, NULL, NULL}, /* unsupported */
	{289, NULL, NULL}, /* unsupported */
	{290, NULL, NULL}, /* unsupported */
	{291, NULL, NULL}, /* unsupported */
	{292, NULL, NULL}, /* unsupported */
	{293, NULL, NULL}, /* unsupported */
	{294, NULL, NULL}, /* unsupported */
	{295, NULL, NULL}, /* unsupported */
	{296, NULL, NULL}, /* unsupported */
	{297, NULL, NULL}, /* unsupported */
	{298, NULL, NULL}, /* unsupported */
	{299, NULL, NULL}, /* unsupported */
	{300, NULL,                                         response_send_rpl_none},
	{301, NULL, NULL}, /* unsupported */
	{302, NULL, NULL},
	{303, NULL, NULL},
	{304, NULL, NULL}, /* unsupported */
	{305, NULL, NULL},
	{306, NULL, NULL},
	{307, NULL, NULL}, /* unsupported */
	{308, NULL, NULL}, /* unsupported */
	{309, NULL, NULL}, /* unsupported */
	{310, NULL, NULL}, /* unsupported */
	{311, NULL, NULL},
	{312, NULL, NULL},
	{313, NULL, NULL},
	{314, NULL, NULL}, /* unsupported */
	{315, NULL, NULL}, /* unsupported */
	{316, NULL, NULL}, /* unsupported */
	{317, NULL, NULL},
	{318, NULL, NULL},
	{319, NULL, NULL},
	{320, NULL, NULL}, /* unsupported */
	{321, NULL, NULL},
	{322, NULL, NULL},
	{323, NULL, NULL},
	{324, NULL, NULL},
	{325, NULL, NULL}, /* unsupported */
	{326, NULL, NULL}, /* unsupported */
	{327, NULL, NULL}, /* unsupported */
	{328, NULL, NULL}, /* unsupported */
	{329, NULL, NULL}, /* unsupported */
	{330, NULL, NULL}, /* unsupported */
	{331, NULL, NULL},
	{332, "<channel> :<topic>",                         response_send_rpl_join},
	{333, NULL, NULL}, /* unsupported */
	{334, NULL, NULL}, /* unsupported */
	{335, NULL, NULL}, /* unsupported */
	{336, NULL, NULL}, /* unsupported */
	{337, NULL, NULL}, /* unsupported */
	{338, NULL, NULL}, /* unsupported */
	{339, NULL, NULL}, /* unsupported */
	{340, NULL, NULL}, /* unsupported */
	{341, NULL, NULL},
	{342, NULL, NULL},
	{343, NULL, NULL}, /* unsupported */
	{344, NULL, NULL}, /* unsupported */
	{345, NULL, NULL}, /* unsupported */
	{346, NULL, NULL}, /* unsupported */
	{347, NULL, NULL}, /* unsupported */
	{348, NULL, NULL}, /* unsupported */
	{349, NULL, NULL}, /* unsupported */
	{350, NULL, NULL}, /* unsupported */
	{351, NULL, NULL},
	{352, NULL, NULL},
	{353, NULL,                                        response_send_rpl_names},
	{354, NULL, NULL}, /* unsupported */
	{355, NULL, NULL}, /* unsupported */
	{356, NULL, NULL}, /* unsupported */
	{357, NULL, NULL}, /* unsupported */
	{358, NULL, NULL}, /* unsupported */
	{359, NULL, NULL}, /* unsupported */
	{360, NULL, NULL}, /* unsupported */
	{361, NULL, NULL}, /* unsupported */
	{362, NULL, NULL}, /* unsupported */
	{363, NULL, NULL}, /* unsupported */
	{364, NULL, NULL},
	{365, NULL, NULL}, /* unsupported */
	{366, NULL, NULL},
	{367, NULL, NULL},
	{368, NULL, NULL},
	{369, NULL, NULL},
	{370, NULL, NULL}, /* unsupported */
	{371, NULL, NULL},
	{372, NULL, NULL}, /* unsupported */
	{373, NULL, NULL}, /* unsupported */
	{374, NULL, NULL},
	{375, NULL, NULL},
	{376, NULL, NULL},
	{377, NULL, NULL}, /* unsupported */
	{378, NULL, NULL}, /* unsupported */
	{379, NULL, NULL}, /* unsupported */
	{380, NULL, NULL}, /* unsupported */
	{381, NULL, NULL},
	{382, NULL, NULL},
	{383, NULL, NULL}, /* unsupported */
	{384, NULL, NULL}, /* unsupported */
	{385, NULL, NULL}, /* unsupported */
	{386, NULL, NULL}, /* unsupported */
	{387, NULL, NULL}, /* unsupported */
	{388, NULL, NULL}, /* unsupported */
	{389, NULL, NULL}, /* unsupported */
	{390, NULL, NULL}, /* unsupported */
	{391, NULL, NULL},
	{392, NULL, NULL},
	{393, NULL, NULL},
	{394, NULL, NULL},
	{395, NULL, NULL},
	{396, NULL, NULL}, /* unsupported */
	{397, NULL, NULL}, /* unsupported */
	{398, NULL, NULL}, /* unsupported */
	{399, NULL, NULL}, /* unsupported */
	{400, NULL, NULL}, /* unsupported */
	{401, "<nick> :No such nick/channel",                    response_send_err},
	{402, NULL, NULL},
	{403, "<channel> :No such channel",                      response_send_err},
	{404, NULL, NULL},
	{405, NULL, NULL},
	{406, NULL, NULL},
	{407, NULL, NULL},
	{408, NULL, NULL}, /* unsupported */
	{409, NULL, NULL},
	{410, NULL, NULL}, /* unsupported */
	{411, ":No recipient given (<command>)",                 response_send_err},
	{412, NULL, NULL},
	{413, NULL, NULL},
	{414, NULL, NULL},
	{415, NULL, NULL}, /* unsupported */
	{416, NULL, NULL}, /* unsupported */
	{417, NULL, NULL}, /* unsupported */
	{418, NULL, NULL}, /* unsupported */
	{419, NULL, NULL}, /* unsupported */
	{420, NULL, NULL}, /* unsupported */
	{421, NULL, NULL},
	{422, NULL, NULL},
	{423, NULL, NULL},
	{424, NULL, NULL},
	{425, NULL, NULL}, /* unsupported */
	{426, NULL, NULL}, /* unsupported */
	{427, NULL, NULL}, /* unsupported */
	{428, NULL, NULL}, /* unsupported */
	{429, NULL, NULL}, /* unsupported */
	{430, NULL, NULL}, /* unsupported */
	{431, ":No nickname given",                              response_send_err},
	{432, "<nick> :Erroneus nickname",                       response_send_err},
	{433, "<nick> :Nickname is already in use",              response_send_err},
	{434, NULL, NULL}, /* unsupported */
	{435, NULL, NULL}, /* unsupported */
	{436, "<nick> :Nickname collision KILL",                 response_send_err},
	{437, NULL, NULL}, /* unsupported */
	{438, NULL, NULL}, /* unsupported */
	{439, NULL, NULL}, /* unsupported */
	{440, NULL, NULL}, /* unsupported */
	{441, NULL, NULL},
	{442, NULL, NULL},
	{443, NULL, NULL},
	{444, NULL, NULL},
	{445, NULL, NULL},
	{446, NULL, NULL},
	{447, NULL, NULL}, /* unsupported */
	{448, NULL, NULL}, /* unsupported */
	{449, NULL, NULL}, /* unsupported */
	{450, NULL, NULL}, /* unsupported */
	{451, NULL, NULL},
	{452, NULL, NULL}, /* unsupported */
	{453, NULL, NULL}, /* unsupported */
	{454, NULL, NULL}, /* unsupported */
	{455, NULL, NULL}, /* unsupported */
	{456, NULL, NULL}, /* unsupported */
	{457, NULL, NULL}, /* unsupported */
	{458, NULL, NULL}, /* unsupported */
	{459, NULL, NULL}, /* unsupported */
	{460, NULL, NULL}, /* unsupported */
	{461, "<%s> :Not enough parameters",                     response_send_err},
	{462, NULL, NULL},
	{463, NULL, NULL},
	{464, NULL, NULL},
	{465, NULL, NULL},
	{466, NULL, NULL}, /* unsupported */
	{467, NULL, NULL},
	{468, NULL, NULL}, /* unsupported */
	{469, NULL, NULL}, /* unsupported */
	{470, NULL, NULL}, /* unsupported */
	{471, NULL, NULL},
	{472, NULL, NULL},
	{473, NULL, NULL},
	{474, NULL, NULL},
	{475, NULL, NULL},
	{476, NULL, NULL}, /* unsupported */
	{477, NULL, NULL}, /* unsupported */
	{478, NULL, NULL}, /* unsupported */
	{479, NULL, NULL}, /* unsupported */
	{480, NULL, NULL}, /* unsupported */
	{481, NULL, NULL},
	{482, NULL, NULL},
	{483, NULL, NULL},
	{484, NULL, NULL}, /* unsupported */
	{485, NULL, NULL}, /* unsupported */
	{486, NULL, NULL}, /* unsupported */
	{487, NULL, NULL}, /* unsupported */
	{488, NULL, NULL}, /* unsupported */
	{489, NULL, NULL}, /* unsupported */
	{490, NULL, NULL}, /* unsupported */
	{491, NULL, NULL},
	{492, NULL, NULL}, /* unsupported */
	{493, NULL, NULL}, /* unsupported */
	{494, NULL, NULL}, /* unsupported */
	{495, NULL, NULL}, /* unsupported */
	{496, NULL, NULL}, /* unsupported */
	{497, NULL, NULL}, /* unsupported */
	{498, NULL, NULL}, /* unsupported */
	{499, NULL, NULL}, /* unsupported */
	{500, NULL, NULL}, /* unsupported */
	{501, NULL, NULL},
	{502, NULL, NULL},
};
