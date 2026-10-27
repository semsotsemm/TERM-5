using ASPA0011_1.Logging;
using ASPA0011_1.Services;

public partial class Program
{
    private static void Main(string[] args)
    {
        var builder = WebApplication.CreateBuilder(args);

        builder.Services.AddControllersWithViews();

        builder.Logging.ClearProviders(); 
        builder.Logging.AddConsole();     
        builder.Logging.AddDebug();       
        builder.Logging.AddFile(Path.Combine(Directory.GetCurrentDirectory(), "app.log")); 


        builder.Services.AddSingleton<ChannelManager>();

        var app = builder.Build();

        if (!app.Environment.IsDevelopment())
        {
            app.UseExceptionHandler("/Home/Error");
            app.UseHsts();
        }

        app.UseHttpsRedirection();
        app.UseRouting();

        app.UseAuthorization();

        app.MapStaticAssets();

        app.MapControllerRoute( 
            name: "default",
            pattern: "{controller=Home}/{action=Index}/{id?}")
            .WithStaticAssets();


        app.Run();
        app.Logger.LogInformation("Остановка приложения ASPA0011_1");
    }
}