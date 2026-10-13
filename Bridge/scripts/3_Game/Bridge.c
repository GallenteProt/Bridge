// Bridge: server-only mod. Collects game data and posts it to the DayZ End agent over HTTP
// (POST <url>bridge/<instance>), then runs the commands the agent returns.

class BridgeConfig
{
	string url = "http://127.0.0.1:7810/";
	string key;
	string instance;
	int posInterval = 3;
	int vehInterval = 10;
	int enabled = 1;
}

class BridgeCmd
{
	string id;
	string op;
	string steam;
	string target;
	string reason;
	string text;
	string title;
	string cls;
	string where;
	string netId;
	float posX;
	float posY;
	float posZ;
	float showTime;
	float wOvercast;
	float wRain;
	float wFog;
	float wWind;
	int hasY;
	int qty;
	int flag;
	int hour;
	int minute;
}

class BridgeReply
{
	int ok;
	ref array<ref BridgeCmd> cmds = new array<ref BridgeCmd>;
}

class BridgeCallback : RestCallback
{
	override void OnSuccess(string data, int dataSize)
	{
		Bridge.Get().OnReply(data);
	}

	override void OnError(int errorCode)
	{
		Bridge.Get().OnFailed();
	}

	override void OnTimeout()
	{
		Bridge.Get().OnFailed();
	}
}

class Bridge
{
	static const int MAX_EVENTS = 300;

	protected static ref Bridge s_Instance;

	protected ref BridgeConfig m_Cfg;
	protected RestContext m_Ctx;
	protected ref BridgeCallback m_Cb;
	protected ref array<string> m_Events = new array<string>;
	protected ref array<ref BridgeCmd> m_Cmds = new array<ref BridgeCmd>;
	protected bool m_Busy;
	protected bool m_Hello;
	protected float m_Timer;

	static Bridge Get()
	{
		if (!s_Instance)
		{
			s_Instance = new Bridge();
			s_Instance.Init();
		}
		return s_Instance;
	}

	protected void Init()
	{
		m_Cfg = new BridgeConfig();
		string file = "$profile:Bridge/bridge.json";
		if (!FileExist(file))
		{
			m_Cfg.enabled = 0;
			return;
		}
		JsonFileLoader<BridgeConfig>.JsonLoadFile(file, m_Cfg);
		if (m_Cfg.enabled == 0 || m_Cfg.key == "")
		{
			m_Cfg.enabled = 0;
			return;
		}
		RestApi api = GetRestApi();
		if (!api)
			api = CreateRestApi();
		m_Ctx = api.GetRestContext(m_Cfg.url);
		m_Ctx.SetHeader("application/json");
		m_Cb = new BridgeCallback();
	}

	bool IsEnabled()
	{
		return m_Cfg && m_Cfg.enabled != 0 && m_Ctx;
	}

	int GetPosInterval()
	{
		return Math.Max(1, m_Cfg.posInterval);
	}

	static string Esc(string s)
	{
		s.Replace("\\", "\\\\");
		s.Replace("\"", "\\\"");
		return s;
	}

	void AddEvent(string json)
	{
		if (!IsEnabled())
			return;
		if (m_Events.Count() >= MAX_EVENTS)
			m_Events.RemoveOrdered(0);
		m_Events.Insert(json);
	}

	void AddResult(string id, bool ok, string err)
	{
		int okv = 0;
		if (ok)
			okv = 1;
		AddEvent("{\"t\":\"res\",\"id\":\"" + Esc(id) + "\",\"ok\":" + okv + ",\"err\":\"" + Esc(err) + "\"}");
	}

	BridgeCmd PopCmd()
	{
		if (m_Cmds.Count() == 0)
			return null;
		BridgeCmd c = m_Cmds.Get(0);
		m_Cmds.RemoveOrdered(0);
		return c;
	}

	// called every frame from the mission; sends a batch once a second
	void Tick(float dt)
	{
		if (!IsEnabled())
			return;
		m_Timer += dt;
		if (m_Timer < 1.0)
			return;
		m_Timer = 0;
		Flush();
	}

	protected string WorldName()
	{
		string w = GetGame().GetWorldName();
		w.ToLower();
		return w;
	}

	protected string Heartbeat()
	{
		int y;
		int mo;
		int d;
		int h;
		int mi;
		GetGame().GetWorld().GetDate(y, mo, d, h, mi);
		string hh = h.ToString();
		if (h < 10)
			hh = "0" + hh;
		string mm = mi.ToString();
		if (mi < 10)
			mm = "0" + mm;
		return "{\"t\":\"hb\",\"fps\":" + Math.Round(GetGame().GetFps()) + ",\"up\":" + Math.Round(GetGame().GetTickTime()) + ",\"gt\":\"" + hh + ":" + mm + "\"}";
	}

	protected void Flush()
	{
		if (m_Busy)
			return;
		string ev = "";
		if (!m_Hello)
			ev = "{\"t\":\"hello\",\"ver\":\"rest-1.0\",\"map\":\"" + WorldName() + "\",\"world\":\"" + WorldName() + "\"}";
		foreach (string e : m_Events)
		{
			if (ev != "")
				ev = ev + ",";
			ev = ev + e;
		}
		m_Events.Clear();
		if (ev != "")
			ev = ev + ",";
		ev = ev + Heartbeat();
		string body = "{\"key\":\"" + Esc(m_Cfg.key) + "\",\"ver\":\"rest-1.0\",\"events\":[" + ev + "]}";
		m_Busy = true;
		m_Ctx.POST(m_Cb, "bridge/" + m_Cfg.instance, body);
	}

	void OnReply(string data)
	{
		m_Busy = false;
		m_Hello = true;
		BridgeReply r = new BridgeReply();
		string err;
		JsonSerializer js = new JsonSerializer();
		if (!js.ReadFromString(r, data, err))
		{
			PrintToRPT("[Bridge] bad reply: " + err);
			return;
		}
		foreach (BridgeCmd c : r.cmds)
			m_Cmds.Insert(c);
	}

	void OnFailed()
	{
		m_Busy = false;
		m_Hello = false; // the agent may have restarted: introduce ourselves again
	}
}
