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

#include <sstream>   // std::istringstream
#include <vector>    // std::vector
#include <cstdlib>   // std::strtol

EGameCommand TeleportCommand::getId()
{
	return eGameCommand_Teleport;
}

void TeleportCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
    // 1. 将字节数组转为字符串（使用 data 和 length，避免迭代器）
    std::string cmdText(reinterpret_cast<const char*>(commandData.data), commandData.length);

    // 2. 分割参数
    std::vector<std::string> args;
    std::istringstream iss(cmdText);
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    // 只支持 /tp <x> <y> <z>（三个整数）
    if (args.size() != 3) {
        return;
    }

    // 3. 确保命令发送者是玩家
    shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(source);
    if (!player) {
        return;
    }

    // 4. 检查玩家是否存活
    if (!player->isAlive()) {
        return;
    }

    // 5. 解析坐标（无边界检查，直接使用 strtol）
    char* endPtrX = nullptr;
    char* endPtrY = nullptr;
    char* endPtrZ = nullptr;

    int x = static_cast<int>(std::strtol(args[0].c_str(), &endPtrX, 10));
    int y = static_cast<int>(std::strtol(args[1].c_str(), &endPtrY, 10));
    int z = static_cast<int>(std::strtol(args[2].c_str(), &endPtrZ, 10));

    // 6. 执行传送（居中到方块中心）
    player->teleportTo(x + 0.5f, y, z + 0.5f);

    // 7. （可选）日志记录
    // logAdminAction(source, "commands.tp.coordinates", player->getName(), x, y, z);
}

shared_ptr<GameCommandPacket> TeleportCommand::preparePacket(PlayerUID subject, PlayerUID destination)
{
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    dos.writePlayerUID(subject);
    dos.writePlayerUID(destination);

    return std::make_shared<GameCommandPacket>(eGameCommand_Teleport, baos.toByteArray());
}
