modded class PlayerBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);
		if (!GetGame().IsServer())
			return;
		Bridge b = Bridge.Get();
		if (!b.IsEnabled())
			return;
		PlayerIdentity vid = GetIdentity();
		if (!vid)
			return;
		vector vp = GetPosition();
		string v = "{\"s\":\"" + vid.GetPlainId() + "\",\"n\":\"" + Bridge.Esc(vid.GetName()) + "\",\"x\":" + vp[0] + ",\"z\":" + vp[2] + "}";

		string k = "{\"kind\":\"unknown\"}";
		string weapon = "";
		float dist = 0;
		if (killer)
		{
			Man km = null;
			EntityAI ke = EntityAI.Cast(killer);
			if (ke)
				km = ke.GetHierarchyRootPlayer();
			if (!km)
				km = Man.Cast(killer);
			PlayerIdentity kid = null;
			if (km)
				kid = km.GetIdentity();
			if (kid)
			{
				vector kp = km.GetPosition();
				dist = vector.Distance(kp, vp);
				if (ke && ke != km)
					weapon = ke.GetType();
				k = "{\"kind\":\"player\",\"s\":\"" + kid.GetPlainId() + "\",\"n\":\"" + Bridge.Esc(kid.GetName()) + "\",\"x\":" + kp[0] + ",\"z\":" + kp[2] + "}";
			}
			else if (ZombieBase.Cast(killer))
				k = "{\"kind\":\"zombie\",\"cls\":\"" + killer.GetType() + "\"}";
			else if (AnimalBase.Cast(killer))
				k = "{\"kind\":\"animal\",\"cls\":\"" + killer.GetType() + "\"}";
			else if (killer == this)
				k = "{\"kind\":\"suicide\"}";
			else
				k = "{\"kind\":\"env\",\"cls\":\"" + killer.GetType() + "\"}";
		}
		b.AddEvent("{\"t\":\"death\",\"v\":" + v + ",\"k\":" + k + ",\"w\":\"" + Bridge.Esc(weapon) + "\",\"d\":" + dist + "}");
	}
}
