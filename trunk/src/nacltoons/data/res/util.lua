-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- Utility functions and constants shared by nacltoons lua code

local util = {}

util.PTM_RATIO = 32

util.tags = {
    TAG_DYNAMIC_START = 0xff,
}

-- Convert a value from screen coordinate system to Box2D world coordinates
function util.ScreenToWorld(value)
    return value / util.PTM_RATIO
end

--- Log messages to console
function util.Log(...)
    print('LUA: '..string.format(...))
end

--- Create CCPoint from a lua table containing 2 elements.
-- This is used to convert point data from .def files into
-- the cocos2dx coordinate space.
function util.PointFromLua(point)
    return CCPointMake(point[1] + game_obj.origin.x, point[2] + game_obj.origin.y)
end

--- Convert CCPoint to b2Vec.
function util.b2VecFromCocos(cocos_vec)
    return b2Vec2:new_local(util.ScreenToWorld(cocos_vec.x),
                            util.ScreenToWorld(cocos_vec.y))
end

return util