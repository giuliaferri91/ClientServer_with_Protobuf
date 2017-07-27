#include "parser.hh"

void split_part(vector<fun>* f, string line)
{
	string delimiters(" ,(){};");
	fun tmp;
	vector<string> parts;
	boost::split(parts, line, boost::is_any_of(delimiters));
	vector<string> new_v(parts.size());
	auto it = copy_if(parts.begin(), parts.end(), new_v.begin(),
						 [](string i) { return !(i == ""); });
	new_v.resize(it - new_v.begin());  	
	tmp.parts = new_v;
	tmp.line = line;
	if(!tmp.parts.empty())
		f->push_back(tmp);
}

void read_declaration(string name, vector<fun>* f, vector<msg_proto>* t)
{
	ifstream file;
	string line;
	int n_par = 0;
	string delimiters(" {");

	file.open(name);
	
	while(getline(file,line))
	{
		if(line.find("message") != string::npos) 
		{
			msg_proto tmp;
			vector<string> l_msg;
			vector<string> parts;
			boost::split(parts, line, boost::is_any_of(delimiters));
			tmp.name = parts[1];
			tmp.n_line = 0;
			tmp.read = false;
			if(line.find("{") != string::npos) 
				n_par++;
			while(getline(file,line))
			{
				if(line.find("{") != string::npos) 
					n_par++;

				if(line.find("}") != string::npos)
				{
					n_par--;	
					break;
				}
				if((line.find("optional") != string::npos) ||
				   (line.find("required") != string::npos))
					tmp.n_line++;
					l_msg.push_back(line);

			}
			tmp.line = l_msg;
			t->push_back(tmp);
		}

		else if(line.find("interface") != string::npos) 
		{
			split_part(f, line);

			if(line.find("{") != string::npos) 
				n_par++;
			while(getline(file,line))
			{
				split_part(f, line);

				if(line.find("{") != string::npos) 
					n_par++;
				if(line.find("}") != string::npos)
				{
					n_par--;	
					break;
				}
			}

		}
		else 
		{
			if(line.find("{") != string::npos) 
				n_par++;
			if(line.find("}") != string::npos)
				n_par--;
			
		}
		if(n_par != 0) 
		{
			cout<<"sintax error: parenthesis mismatched"<<endl;
			exit(0);
		}

	}

	file.clear();
	file.seekg(0,ios::beg);

	file.close();

}

string write_proto(vector<fun>* f, vector<msg_proto>* t)
{
	ofstream protobuf;
	protobuf.open("./AutoGenerated/fun.proto");
	string Name, s;
	int count;

	for(msg_proto& x: *t)
	{
		protobuf<< "message " << x.name << " {"<<'\n';
		for(string& s: x.line)
			protobuf<<s <<"\n";
		protobuf<< "};\n";
	}

	for(fun& x: *f) 
	{
		int id = 1;	
		fun tmp = x;
		if(tmp.parts[0] == "interface")															
			Name = tmp.parts[1];
		else {
			replace (tmp.parts.begin(), tmp.parts.end(), string("int"), string("sint32")); 
			replace (tmp.parts.begin(), tmp.parts.end(), string("char"), string("string")); 
			replace (tmp.parts.begin(), tmp.parts.end(), string("long"), string("sint64"));
			protobuf<< "message m_" << tmp.parts[1] << " {"<<'\n';
			if(tmp.parts[0] != "void"){
				protobuf<<"optional "<<tmp.parts[0]<<" response = "<< id <<" ;\n";
				id++;
			}
			for (int i=2; i<tmp.parts.size(); i=i+2){
				protobuf<<"optional "<<tmp.parts[i]<<" "<< tmp.parts[i+1]<< " = "<<id<<" ;\n";
				id++;
			}
			protobuf<< "};\n";
		}
	}	
	
	protobuf.close();
	string newname = "./AutoGenerated/" + Name + ".proto";
	char newfile[newname.size()+1];//as 1 char space for null is also required
	strcpy(newfile, newname.c_str());

    int rc = rename("./AutoGenerated/fun.proto", newfile); 
	return Name;
}


void write_class_hh(string name, vector<fun> f)
{
	ifstream  lib;
	ofstream class_h;
	int count;
	string stringFunctionVirtual="", stringFunction ="";
	string stringEnum = "enum fun {";	
	string code;
	string ToReplace = "autoserial";
	string ToReplace2 = "--virtual1--";
	string ToReplace3 = "--virtual2--";
	string ToReplace4 = "--include--";
	string ToReplace5 = "--enum--";
	std::vector<fun>::iterator it;

	lib.open("autoserial.hh");
	class_h.open("./AutoGenerated/"+name+".hh");

	for (fun& x: f)
		if(!x.parts.empty() && x.parts[0] != "interface")
		{
			stringFunctionVirtual += "virtual " + x.line + "\n\t\t";
			stringFunction += x.line + "\n\t\t";
			stringEnum += "f_" + x.parts[1] + ",";			
		}

	boost::replace_last(stringEnum, ",", "};");
	boost::replace_all(stringFunctionVirtual, ";", " = 0;");

	while(getline(lib,code))
	{
		boost::replace_all(code, ToReplace4, "#include \"" +name+".pb.h\"");
		boost::replace_all(code, ToReplace5, stringEnum);
		boost::replace_all(code, ToReplace, name);
		boost::replace_all(code, ToReplace2, stringFunctionVirtual);
		boost::replace_all(code, ToReplace3, stringFunction);
		class_h<<code<<'\n';
	}

	lib.close();
	class_h.close();
}

void write_class_cc(string name, vector<fun> f, std::vector<msg_proto> t)
{
	ifstream source;
	ofstream class_c;
	string code;
	string ToReplace = "autoserial";
	string client = "", server = "";

	source.open("autoserial.cc");
	class_c.open("./AutoGenerated/"+name+".cc");

	for (fun& x: f)
		if(!x.parts.empty() && x.parts[0] != "interface"){
			string setproto = "", call_function ="";
			boost::replace_all(x.line, ";", "");
			client += x.line;
			boost::replace_all(client, x.parts[1], "client::"+ x.parts[1]);
			client += CLIENT_IMPL(x.parts[1]);
			for(int i = 3; i<x.parts.size() ; i=i+2)
			{
				bool is_msg=false;
				for(msg_proto& m: t)
					if(x.parts[i-1] == m.name) 
					{
						is_msg = true;
						break;
					}	
				if(is_msg)
					setproto += "\tproto_msg.mutable_" + x.parts[i] + "()->CopyFrom(" + x.parts[i] + ");\n";
				else 
					setproto += "\tproto_msg.set_" + x.parts[i] + "(" + x.parts[i] + ");\n";
			}
			boost::replace_all(client, "--fill_msg--", setproto);

			call_function += x.parts[0] + " r = " + x.parts[1] +"(";
			server += SERVER_IMPL(x.parts[1]);
			for(int i = 3; i<x.parts.size() ; i=i+2)
				call_function += "proto_msg."+ x.parts[i] + "()," ;
			call_function += ");\n";
			boost::replace_last(call_function, ",", "");
			
			bool is_msg=false;
			for(msg_proto& m: t)
				if(x.parts[0] == m.name) 
				{
					is_msg = true;
					break;
				}	
			if(is_msg)
				call_function += "\t\t\t\tproto_msg.mutable_response()->CopyFrom(r);";
			else 
				call_function += "\t\t\t\tproto_msg.set_response(r);";
			for(int i = 3; i<x.parts.size() ; i=i+2)
				call_function += "\n\t\t\t\tproto_msg.clear_"+ x.parts[i] + "();" ;
			boost::replace_all(server, "--call_function--", call_function);
		}


	while(getline(source,code))
	{
		boost::replace_all(code, ToReplace, name);
		boost::replace_all(code, "--function--", client);
		boost::replace_all(code, "--function_switch--", server);
		class_c<<code<<'\n';
	}

	source.close();
	class_c.close();
}

void write_make(string name)
{
	ifstream makein;
	ofstream makeout;
	makein.open("Makefile.x");
	makeout.open("./AutoGenerated/Makefile");
	string line;

	while(getline(makein,line)){
		boost::replace_all(line, "test", name);
		makeout<<line<<'\n';
	}

	makein.close();
	makeout.close();
}

int main(int argc, char* argv[])
{
	string NameClass;
	vector<fun> f;
	vector<msg_proto> t;

	if(argc < 2)
		cout<<"missing file .pbc -> e.g.: ./parser filename.pbc"<<endl;
	else 
	{
		string delimiters(".");
		vector<string> parts;
		boost::split(parts, argv[1], boost::is_any_of(delimiters));
		if(parts[1] == "pbc")
		{
			system("mkdir -p ./AutoGenerated");
			read_declaration(argv[1], &f, &t);
			NameClass = write_proto(&f, &t);
			write_class_hh(NameClass, f);
			write_class_cc(NameClass, f, t);
			write_make(NameClass);
		}
		else 
			cout<<"extension error -> filename.pbc"<<endl;
	}
	return 0;
}

