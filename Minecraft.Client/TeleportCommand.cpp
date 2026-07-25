#include "stdafx.h"
#include "MinecraftServer.h"
#include "PlayerList.h"
#include "ServerPlayer.h"
#include "PlayerConnection.h"
#include "../Minecraft.World/net.minecraft.commands.h"
#include "../Minecraft.World/net.minecraft.network.packet.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "TeleportCommand.h"

#include <cstdlib>   // 用于 std::strtol
#include <sstream>   // 用于 std::istringstream
#include <vector>    // 用于 std::vector

EGameCommand TeleportCommand::getId()
{
	return eGameCommand_Teleport;
}

void TeleportCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
    // 1. 将字节数组转为字符串并分割参数
    std::string cmdText(commandData.begin(), commandData.end());
    std::vector<std::string> args;
    std::istringstream iss(cmdText);
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    // 只支持 /tp <x> <y> <z>
    if (args.size() != 3) {
        return;
    }

    // 2. 确保命令发送者是玩家
    shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(source);
    if (!player) {
        return;
    }

    // 3. 检查玩家是否存活
    if (!player->isAlive()) {
        return;
    }

    // 4. 解析坐标（无任何边界限制）
    char* endPtrX = nullptr;
    char* endPtrY = nullptr;
    char* endPtrZ = nullptr;

    int x = static_cast<int>(std::strtol(args[0].c_str(), &endPtrX, 10));
    int y = static_cast<int>(std::strtol(args[1].c_str(), &endPtrY, 10));
    int z = static_cast<int>(std::strtol(args[2].c_str(), &endPtrZ, 10));

    // 5. 执行传送（居中到方块中心）
    player->teleportTo(x + 0.5f, y, z + 0.5f);

    // 6. （可选）日志记录
    // logAdminAction(source, "commands.tp.coordinates", player->getName(), x, y, z);
}
