modded class MissionServer
{
	protected float m_BridgePosTimer;

	override void OnInit()
	{
		super.OnInit();
		Bridge.Get();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		Bridge b = Bridge.Get();
		if (!b.IsEnabled())
			return;

		m_BridgePosTimer += timeslice;
		if (m_BridgePosTimer >= b.GetPosInterval())
		{
			m_BridgePosTimer = 0;
			BridgeSendPositions(b);
		}

		BridgeCmd cmd = b.PopCmd();
		int guard = 0;
		while (cmd && guard < 20)
		{
			BridgeRunCommand(b, cmd);
			cmd = b.PopCmd();
			guard++;
		}

		b.Tick(timeslice);
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);
		Bridge b = Bridge.Get();
		if (b.IsEnabled() && identity)
			b.AddEvent("{\"t\":\"connect\",\"s\":\"" + identity.GetPlainId() + "\",\"n\":\"" + Bridge.Esc(identity.GetName()) + "\",\"id\":\"" + identity.GetId() + "\"}");
	}

	override void InvokeOnDisconnect(PlayerBase player)
	{
		PlayerIdentity identity = null;
		if (player)
			identity = player.GetIdentity();
		Bridge b = Bridge.Get();
		if (b.IsEnabled() && identity)
			b.AddEvent("{\"t\":\"disconnect\",\"s\":\"" + identity.GetPlainId() + "\",\"n\":\"" + Bridge.Esc(identity.GetName()) + "\"}");
		super.InvokeOnDisconnect(player);
	}

	protected void BridgeSendPositions(Bridge b)
	{
		array<Man> men = new array<Man>;
		GetGame().GetPlayers(men);
		string rows = "";
		foreach (Man m : men)
		{
			PlayerBase pb = PlayerBase.Cast(m);
			if (!pb)
				continue;
			PlayerIdentity id = pb.GetIdentity();
			if (!id)
				continue;
			vector pos = pb.GetPosition();
			vector ori = pb.GetOrientation();
			int alive = 0;
			if (pb.IsAlive())
				alive = 1;
			if (rows != "")
				rows = rows + ",";
			rows = rows + "{\"s\":\"" + id.GetPlainId() + "\",\"n\":\"" + Bridge.Esc(id.GetName()) + "\",\"x\":" + pos[0] + ",\"y\":" + pos[1] + ",\"z\":" + pos[2] + ",\"d\":" + ori[0] + ",\"hp\":" + Math.Round(pb.GetHealth("GlobalHealth", "Health")) + ",\"bl\":" + Math.Round(pb.GetHealth("GlobalHealth", "Blood")) + ",\"al\":" + alive + "}";
		}
		// "full": the list is a complete snapshot, the agent drops players that are not in it
		b.AddEvent("{\"t\":\"pos\",\"full\":1,\"p\":[" + rows + "]}");
	}

	protected PlayerBase BridgeFindPlayer(string steam)
	{
		array<Man> men = new array<Man>;
		GetGame().GetPlayers(men);
		foreach (Man m : men)
		{
			PlayerIdentity id = m.GetIdentity();
			if (id && id.GetPlainId() == steam)
				return PlayerBase.Cast(m);
		}
		return null;
	}

	protected void BridgeNotify(Man to, BridgeCmd c)
	{
		float t = c.showTime;
		if (t <= 0)
			t = 10;
		NotificationSystem.SendNotificationToPlayerExtended(to, t, c.title, c.text, "set:dayz_gui image:icon_info");
	}

	protected void BridgeRunCommand(Bridge b, BridgeCmd c)
	{
		PlayerBase pb = null;
		if (c.steam != "")
			pb = BridgeFindPlayer(c.steam);

		switch (c.op)
		{
			case "shutdown":
				b.AddResult(c.id, true, "");
				GetGame().RequestExit(0);
				return;

			case "notify":
			case "msg":
			{
				if (c.target == "all" || (c.steam == "" && c.op == "notify"))
				{
					array<Man> men = new array<Man>;
					GetGame().GetPlayers(men);
					foreach (Man m : men)
						BridgeNotify(m, c);
					b.AddResult(c.id, true, "");
				}
				else if (pb)
				{
					BridgeNotify(pb, c);
					b.AddResult(c.id, true, "");
				}
				else
					b.AddResult(c.id, false, "no_player");
				return;
			}

			case "kick":
				if (pb && pb.GetIdentity())
				{
					GetGame().DisconnectPlayer(pb.GetIdentity());
					b.AddResult(c.id, true, "");
				}
				else
					b.AddResult(c.id, false, "no_player");
				return;

			case "tp":
			{
				if (!pb)
				{
					b.AddResult(c.id, false, "no_player");
					return;
				}
				vector dst;
				if (c.target != "")
				{
					PlayerBase other = BridgeFindPlayer(c.target);
					if (!other)
					{
						b.AddResult(c.id, false, "no_target");
						return;
					}
					dst = other.GetPosition();
				}
				else
				{
					dst[0] = c.posX;
					dst[2] = c.posZ;
					if (c.hasY != 0)
						dst[1] = c.posY;
					else
						dst[1] = GetGame().SurfaceY(c.posX, c.posZ);
				}
				pb.SetPosition(dst);
				b.AddResult(c.id, true, "");
				return;
			}

			case "heal":
				if (pb)
				{
					BridgeHeal(pb);
					b.AddResult(c.id, true, "");
				}
				else
					b.AddResult(c.id, false, "no_player");
				return;

			case "healall":
			{
				array<Man> all = new array<Man>;
				GetGame().GetPlayers(all);
				foreach (Man hm : all)
				{
					PlayerBase hp = PlayerBase.Cast(hm);
					if (hp)
						BridgeHeal(hp);
				}
				b.AddResult(c.id, true, "");
				return;
			}

			case "kill":
				if (pb)
				{
					pb.SetHealth("GlobalHealth", "Health", 0);
					b.AddResult(c.id, true, "");
				}
				else
					b.AddResult(c.id, false, "no_player");
				return;
		}
		b.AddResult(c.id, false, "unsupported");
	}

	protected void BridgeHeal(PlayerBase pb)
	{
		pb.SetHealth("GlobalHealth", "Health", pb.GetMaxHealth("GlobalHealth", "Health"));
		pb.SetHealth("GlobalHealth", "Blood", pb.GetMaxHealth("GlobalHealth", "Blood"));
		pb.SetHealth("GlobalHealth", "Shock", pb.GetMaxHealth("GlobalHealth", "Shock"));
	}
}
